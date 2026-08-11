#include "planningthread.h"
#include <QDebug>
#include <chrono>
#include <cmath>
#include <cstdlib>

static const double LIGHT_DETECTION_RANGE = 80.0;

PlanningThread::PlanningThread(sharedDatapool &pool, int vehicleId, QObject *parent)
    : QObject(parent), pool_(pool), vehicleId_(vehicleId),
    parkingActive_(false), passedTrafficLight_(false)
{
}

PlanningThread::~PlanningThread()
{
    stop();
    wait();
}

void PlanningThread::start()
{
    running_ = true;
    thread_ = std::thread(&PlanningThread::run, this);
}

void PlanningThread::stop()
{
    running_ = false;
    parkingActive_ = false;
    passedTrafficLight_ = false;
}

void PlanningThread::wait()
{
    if (thread_.joinable()) {
        thread_.join();
    }
}

void PlanningThread::run()
{
    qDebug() << "[规划] 车辆" << vehicleId_ << " 启动（避让版）";

    int cycle = 0;

    while (running_) {
        cycle++;

        VehicleState state = pool_.getVehicleState(vehicleId_);
        if (state.id != vehicleId_) {
            qDebug() << "[规划] 车辆" << vehicleId_ << " 状态无效，等待...";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // 红绿灯检测
        Trafficlight tl = pool_.getTrafficLight();
        double dx = tl.x - state.x;
        double dy = tl.y - state.y;
        double distToLight = sqrt(dx * dx + dy * dy);
        bool ignoreTrafficLight = (state.x > 110.0);
        bool redLight = (!ignoreTrafficLight && distToLight < LIGHT_DETECTION_RANGE && tl.state == TrafficlightState::Red);
        bool yellowLight = (!ignoreTrafficLight && distToLight < LIGHT_DETECTION_RANGE && tl.state == TrafficlightState::Yellow);

        // 行人检测
        Pedestrian ped = pool_.getPedestrian();
        double dxPed = ped.x - state.x;
        double dyPed = ped.y - state.y;
        double distToPed = sqrt(dxPed * dxPed + dyPed * dyPed);
        bool pedestrianNear = (distToPed < 12.0 && fabs(dyPed) < 15.0);

        // 障碍物检测
        QList<Obstacle> obstacles = pool_.getObstacles(vehicleId_);
        const double DETECTION_RANGE = 60.0;
        const double ANGLE_RANGE = 80.0 * M_PI / 180.0;
        const double SAFE_DISTANCE = 15.0;
        const double EMERGENCY_DIST = 2.0;

        struct ObsInfo { double dist, angle; };
        QList<ObsInfo> frontObs;
        for (const Obstacle& obs : obstacles) {
            double dx2 = obs.x - state.x;
            double dy2 = obs.y - state.y;
            double dist = sqrt(dx2 * dx2 + dy2 * dy2);
            if (dist > DETECTION_RANGE || dist < 0.5) continue;
            double angle = atan2(dy2, dx2) - state.yaw;
            while (angle > M_PI) angle -= 2 * M_PI;
            while (angle < -M_PI) angle += 2 * M_PI;
            if (fabs(angle) < ANGLE_RANGE) {
                frontObs.append({dist, angle});
            }
        }

        double minDist = DETECTION_RANGE + 1.0;
        double minAngle = 0.0;
        bool obstacleDetected = false;
        for (const ObsInfo& info : frontObs) {
            if (info.dist < minDist) {
                minDist = info.dist;
                minAngle = info.angle;
                obstacleDetected = true;
            }
        }

        // 停车检测（车辆1专用）
        bool isParking = false;
        if (vehicleId_ == 1) {
            ParkingSlot slot = pool_.getParkingSlot();
            if (parkingActive_) {
                isParking = true;
            } else {
                if (state.x > 150.0) {
                    double ddx = state.x - slot.x;
                    double ddy = state.y - slot.y;
                    double distToSlot = sqrt(ddx*ddx + ddy*ddy);
                    double latError = fabs(state.y - slot.y);
                    if (distToSlot < 8.0 && latError < 2.0 && !slot.occupied) {
                        isParking = true;
                        parkingActive_ = true;
                        pool_.setParkingSlotOccupied(true);
                        qDebug() << "[规划] 车辆1 触发停车！距离:" << distToSlot;
                    }
                }
            }
        }

        // ----- 决策 -----
        ControlCommand cmd;
        cmd.vehicleId = vehicleId_;
        cmd.targetSpeed = 5.0;
        cmd.targetSteering = 0.0;
        cmd.reason = "正常行驶";
        cmd.resetYaw = false;

        // 1. 红灯（最高优先级）
        if (redLight) {
            cmd.targetSpeed = 0.0;
            cmd.targetSteering = 0.0;
            cmd.reason = "🚦 红灯停车";
        }
        // 2. 行人
        else if (pedestrianNear) {
            if (distToPed < 4.0) {
                cmd.targetSpeed = 0.0;
                cmd.targetSteering = 0.0;
                cmd.reason = "🚶 紧急停车（行人）";
            } else {
                cmd.targetSpeed = 2.0;
                cmd.targetSteering = 0.0;
                cmd.reason = "🚶 减速避让行人";
            }
        }
        // 3. 黄灯
        else if (yellowLight) {
            cmd.targetSpeed = 3.0;
            cmd.targetSteering = 0.0;
            cmd.reason = "🚦 黄灯减速";
        }
        // 4. 停车
        else if (isParking) {
            cmd.targetSpeed = 0.0;
            cmd.targetSteering = 0.0;
            cmd.reason = "🅿️ 已停稳";
        }
        // 5. 障碍物避让
        else if (obstacleDetected && minDist < SAFE_DISTANCE) {
            if (minDist < EMERGENCY_DIST) {
                cmd.targetSpeed = 0.0;
                cmd.targetSteering = 0.0;
                cmd.reason = "🚨 紧急停车（障碍物 " + QString::number(minDist, 'f', 1) + "m）";
            } else {
                // 强制转向
                double steeringAmount;
                if (fabs(minAngle) < 0.05) {
                    steeringAmount = (rand() % 2 == 0) ? 0.6 : -0.6;
                } else {
                    steeringAmount = -minAngle / ANGLE_RANGE * 1.0;
                    if (fabs(steeringAmount) < 0.3) {
                        steeringAmount = (steeringAmount >= 0) ? 0.3 : -0.3;
                    }
                }
                cmd.targetSteering = steeringAmount;
                cmd.targetSpeed = 2.5;
                cmd.reason = "🔄 避让（障碍物 " + QString::number(minDist, 'f', 1) + "m）";
            }
        }
        // 6. 远距离障碍物
        else if (obstacleDetected) {
            cmd.targetSpeed = 3.0;
            cmd.targetSteering = 0.0;
            cmd.reason = "👀 前方障碍物（" + QString::number(minDist, 'f', 1) + "m）- 提前减速";
        }
        // 7. 正常行驶
        else {
            cmd.targetSpeed = 5.0;
            cmd.targetSteering = 0.0;
            cmd.reason = "正常行驶";
        }

        pool_.setControlCommand(cmd);

        if (cycle % 20 == 0) {
            qDebug() << "[规划] 车辆" << vehicleId_
                     << " 决策:" << cmd.reason
                     << " 转向:" << cmd.targetSteering
                     << " 速度:" << cmd.targetSpeed;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    qDebug() << "[规划] 车辆" << vehicleId_ << " 退出";
    emit finished();
}