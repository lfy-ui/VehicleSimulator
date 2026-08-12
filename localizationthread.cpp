#include "localizationthread.h"
#include <QDebug>
#include <chrono>
#include <cmath>

LocalizationThread::LocalizationThread(sharedDatapool &pool, int vehicleId, QObject *parent)
    : QObject(parent), pool_(pool), vehicleId_(vehicleId)
{
}

LocalizationThread::~LocalizationThread()
{
    stop();
    wait();
}

void LocalizationThread::start()
{
    running_ = true;
    thread_ = std::thread(&LocalizationThread::run, this);
}

void LocalizationThread::stop()
{
    running_ = false;
}

void LocalizationThread::wait()
{
    if (thread_.joinable()) {
        thread_.join();
    }
}

void LocalizationThread::run()
{
    qDebug() << "[定位] 车辆" << vehicleId_ << " 启动（持续转向屏蔽）";

    VehicleState state = pool_.getVehicleState(vehicleId_);
    if (state.id != vehicleId_) {
        state.id = vehicleId_;
        state.x = -70.0;
        state.y = (vehicleId_ == 1) ? -20.0 : 20.0;
        state.speed = 0.0;
        state.yaw = 0.0;
    }

    double currentSpeed = 0.0;
    double currentSteering = 0.0;
    int cycle = 0;

    const double MAX_ACCEL = 0.08;
    const double MAX_DECEL = 0.15;
    const double MAX_SPEED = 8.0;
    const double MIN_SPEED = 0.0;

    const double ROAD_CENTER = (vehicleId_ == 1) ? -20.0 : 20.0;
    const double STEERING_GAIN = 0.25;

    // 航向计时归零参数
    const double YAW_THRESHOLD = 1.47;
    const int YAW_TIMER_DURATION = 60;
    bool yawTimerActive = false;
    int yawTimerCounter = 0;

    // 持续屏蔽机制
    bool steeringBlockActive = false;
    int steeringBlockCounter = 0;
    const int MAX_BLOCK_FRAMES = 100;

    while (running_) {
        cycle++;

        ControlCommand cmd = pool_.getControlCommand(vehicleId_);

        // 速度控制
        if (currentSpeed < cmd.targetSpeed) {
            currentSpeed += MAX_ACCEL;
            if (currentSpeed > cmd.targetSpeed) currentSpeed = cmd.targetSpeed;
        } else if (currentSpeed > cmd.targetSpeed) {
            currentSpeed -= MAX_DECEL;
            if (currentSpeed < cmd.targetSpeed) currentSpeed = cmd.targetSpeed;
        }
        currentSpeed = qMax(MIN_SPEED, qMin(MAX_SPEED, currentSpeed));

        //转向控制：如果屏蔽激活，强制转向为 0
        if (steeringBlockActive) {
            // 强制转向为 0
            currentSteering = 0.0;
            steeringBlockCounter++;

            // 检查条件：如果数据池中的目标转向已经为 0，或者超时，解除屏蔽
            if (cmd.targetSteering == 0.0 || steeringBlockCounter > MAX_BLOCK_FRAMES) {
                steeringBlockActive = false;
                steeringBlockCounter = 0;
                qDebug() << "[转向屏蔽] 车辆" << vehicleId_
                         << " 解除转向屏蔽（原因:"
                         << (cmd.targetSteering == 0.0 ? "目标转向归零" : "超时") << "）";
                // 解除后，立即读取当前转向指令
                currentSteering = cmd.targetSteering;
                if (currentSteering > 0.8) currentSteering = 0.8;
                if (currentSteering < -0.8) currentSteering = -0.8;
            }
        } else {
            // 正常读取转向指令
            currentSteering = cmd.targetSteering;
            if (currentSteering > 0.8) currentSteering = 0.8;
            if (currentSteering < -0.8) currentSteering = -0.8;
        }

        // 物理更新（位置）
        state.x += currentSpeed * cos(state.yaw) * 0.1;
        state.y += currentSpeed * sin(state.yaw) * 0.1;

        // 航向更新
        state.yaw += currentSteering * STEERING_GAIN;

        // 航向计时归零逻辑
        if (!yawTimerActive && fabs(state.yaw) >= YAW_THRESHOLD) {
            yawTimerActive = true;
            yawTimerCounter = 0;
            qDebug() << "[航向计时] 车辆" << vehicleId_
                     << " 航向达到阈值:" << state.yaw
                     << " 开始计时3秒";
        }

        if (yawTimerActive) {
            yawTimerCounter++;
            if (yawTimerCounter >= YAW_TIMER_DURATION) {
                // 强制归零
                state.yaw = 0.0;
                // 清空当前转向
                currentSteering = 0.0;
                // 清空数据池中的转向指令
                ControlCommand newCmd = pool_.getControlCommand(vehicleId_);
                newCmd.targetSteering = 0.0;
                pool_.setControlCommand(newCmd);
                steeringBlockActive = true;
                steeringBlockCounter = 0;
                yawTimerActive = false;
                yawTimerCounter = 0;
                qDebug() << "[航向计时] 车辆" << vehicleId_
                         << " 计时结束，航向归零，激活转向屏蔽";
            }
        }

        // 虚拟边界
        double halfWidth = (state.x < 110.0) ? 5.0 : 20.0;
        if (state.y > ROAD_CENTER + halfWidth) {
            state.y = ROAD_CENTER + halfWidth;
        } else if (state.y < ROAD_CENTER - halfWidth) {
            state.y = ROAD_CENTER - halfWidth;
        }

        state.speed = currentSpeed;
        state.id = vehicleId_;

        pool_.updateVehicleState(state);

        if (cycle % 60 == 0) {
            qDebug() << "[定位] 车辆" << vehicleId_
                     << " x=" << state.x
                     << " y=" << state.y
                     << " speed=" << currentSpeed
                     << " steering=" << currentSteering
                     << " yaw=" << state.yaw
                     << " 计时:" << (yawTimerActive ? QString::number(yawTimerCounter) : "未激活")
                     << " 屏蔽:" << (steeringBlockActive ? "激活" : "关闭")
                     << " 半宽:" << halfWidth;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    qDebug() << "[定位] 车辆" << vehicleId_ << " std::thread 退出";
    emit finished();
}