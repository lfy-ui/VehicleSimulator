#include "shareddatapool.h"
#include <cmath>
#include <QThread>

sharedDatapool::sharedDatapool() {
    running = true;
    hasNewDate = false;

    trafficLight_.id = 1;
    trafficLight_.x = 100.0;
    trafficLight_.y = -45.0;
    trafficLight_.state = TrafficlightState::Green;
    trafficLight_.switchInterval = 90;
    trafficLight_.timer = 0;

    addVehicle(1, -70.0, -20.0);
    addVehicle(2, -70.0, 20.0);
    qDebug() << "[数据池] 车辆1 初始位置: (-70, -20)";
    qDebug() << "[数据池] 车辆2 初始位置: (-70, 20)";

    pedestrian_.id = 1;
    pedestrian_.x = 110.0;
    pedestrian_.y = 0.0;
    pedestrian_.speed = 0.8;
    pedestrian_.movingRight = true;

    //初始化停车位（在车道1外侧，y < -40）
    parkingSlot_.id = 1;
    parkingSlot_.x = 170.0;
    parkingSlot_.y = -40.0;
    parkingSlot_.width = 20.0;
    parkingSlot_.height = 8.0;
    parkingSlot_.occupied = false;

    qDebug() << "[数据池] 创建了" << vehicles_.size() << "辆车，行人已初始化，停车位已创建";
}

// ===== 添加车辆 =====
void sharedDatapool::addVehicle(int id, double x, double y)
{
    QMutexLocker locker(&mtx_);
    VehicleState v;
    v.id = id;
    v.x = x;
    v.y = y;
    v.speed = 0.0;
    v.yaw = 0.0;
    vehicles_.append(v);

    ControlCommand cmd;
    cmd.vehicleId = id;
    cmd.targetSpeed = 5.0;
    cmd.targetSteering = 0.0;
    commands_.append(cmd);
}

// ===== 更新车辆状态（按ID） =====
void sharedDatapool::updateVehicleState(const VehicleState &s)
{
    QMutexLocker locker(&mtx_);
    for (int i = 0; i < vehicles_.size(); i++) {
        if (vehicles_[i].id == s.id) {
            vehicles_[i] = s;
            hasNewDate = true;
            sen_.enqueue(1);
            qDebug() << "[写入]" << s.toString();
            cv_.wakeOne();
            return;
        }
    }
}

// ===== 获取单个车辆状态（按ID） =====
VehicleState sharedDatapool::getVehicleState(int id) const
{
    QMutexLocker locker(&mtx_);
    for (const VehicleState& v : vehicles_) {
        if (v.id == id) {
            return v;
        }
    }
    VehicleState empty;
    empty.id = id;
    empty.x = 0.0;
    empty.y = (id == 1) ? -20.0 : 20.0;
    empty.speed = 0.0;
    empty.yaw = 0.0;
    qDebug() << "[数据池] 警告: 未找到车辆" << id << "，返回默认位置";
    return empty;
}

// ===== 获取所有车辆状态 =====
QList<VehicleState> sharedDatapool::getAllVehicleStates() const
{
    QMutexLocker locker(&mtx_);
    return vehicles_;
}

// ===== 兼容：更新车辆1 =====
void sharedDatapool::update(const VehicleState &s)
{
    QMutexLocker locker(&mtx_);
    for (int i = 0; i < vehicles_.size(); i++) {
        if (vehicles_[i].id == 1) {
            vehicles_[i].x = s.x;
            vehicles_[i].y = s.y;
            vehicles_[i].speed = s.speed;
            vehicles_[i].yaw = s.yaw;
            hasNewDate = true;
            sen_.enqueue(1);
            qDebug() << "[写入]" << vehicles_[i].toString();
            cv_.wakeOne();
            return;
        }
    }
    VehicleState newState = s;
    newState.id = 1;
    vehicles_.append(newState);
    hasNewDate = true;
    sen_.enqueue(1);
    qDebug() << "[写入]" << newState.toString();
    cv_.wakeOne();
}

// ===== 兼容：获取车辆1 =====
VehicleState sharedDatapool::get()
{
    QMutexLocker locker(&mtx_);
    for (const VehicleState& v : vehicles_) {
        if (v.id == 1) {
            return v;
        }
    }
    return VehicleState();
}

// ===== addObstacle =====
void sharedDatapool::addObstacle(const Obstacle &obs)
{
    QMutexLocker locker(&mtx_);
    obstacles_.append(obs);
    qDebug() << "[数据池] 车辆" << obs.vehicleId << " 添加障碍物 ID:" << obs.id;
}

// ===== 获取所有障碍物 =====
QList<Obstacle> sharedDatapool::getObstacles() const
{
    QMutexLocker locker(&mtx_);
    return obstacles_;
}

// ===== 获取指定车辆的障碍物 =====
QList<Obstacle> sharedDatapool::getObstacles(int vehicleId) const
{
    QMutexLocker locker(&mtx_);
    QList<Obstacle> result;
    for (const Obstacle& obs : obstacles_) {
        if (obs.vehicleId == vehicleId) {
            result.append(obs);
        }
    }
    return result;
}

// ===== 系统控制 =====
void sharedDatapool::stopSystem()
{
    QMutexLocker locker(&mtx_);
    running = false;
    cv_.wakeAll();
}

bool sharedDatapool::isRunning() const
{
    QMutexLocker locker(&mtx_);
    return running;
}

// ===== 设置控制指令 =====
void sharedDatapool::setControlCommand(const ControlCommand &cmd)
{
    QMutexLocker locker(&mtx_);
    for (int i = 0; i < commands_.size(); i++) {
        if (commands_[i].vehicleId == cmd.vehicleId) {
            commands_[i] = cmd;
            return;
        }
    }
    commands_.append(cmd);
}

// ===== 兼容：获取车辆1的控制指令 =====
ControlCommand sharedDatapool::getControlCommand() const
{
    QMutexLocker locker(&mtx_);
    for (const ControlCommand& cmd : commands_) {
        if (cmd.vehicleId == 1) return cmd;
    }
    ControlCommand defaultCmd;
    defaultCmd.vehicleId = 1;
    return defaultCmd;
}

// ===== 获取指定车辆的控制指令 =====
ControlCommand sharedDatapool::getControlCommand(int vehicleId) const
{
    QMutexLocker locker(&mtx_);
    for (const ControlCommand& cmd : commands_) {
        if (cmd.vehicleId == vehicleId) {
            return cmd;
        }
    }
    ControlCommand defaultCmd;
    defaultCmd.vehicleId = vehicleId;
    defaultCmd.targetSpeed = 5.0;
    defaultCmd.targetSteering = 0.0;
    defaultCmd.reason = "正常行驶";
    qDebug() << "[数据池] 警告: 未找到车辆" << vehicleId << "的控制指令，返回默认值";
    return defaultCmd;
}

void sharedDatapool::cleanObstacle()
{
    this->cleanObstacle(1);
}

void sharedDatapool::cleanObstacle(int vehicleId)
{
    QMutexLocker locker(&mtx_);
    if (vehicleId == -1) {
        obstacles_.clear();
        return;
    }
    QList<Obstacle> remaining;
    for (const Obstacle& o : obstacles_) {
        if (o.vehicleId != vehicleId) {
            remaining.append(o);
        }
    }
    obstacles_ = remaining;
}

// ===== 红绿灯 =====
void sharedDatapool::setTrafficLight(const Trafficlight& tl)
{
    QMutexLocker locker(&mtx_);
    trafficLight_ = tl;
}

Trafficlight sharedDatapool::getTrafficLight() const
{
    QMutexLocker locker(&mtx_);
    return trafficLight_;
}

void sharedDatapool::updateTrafficLight()
{
    QMutexLocker locker(&mtx_);
    trafficLight_.update();
}

// ===== 行人 =====
void sharedDatapool::setPedestrian(const Pedestrian &ped)
{
    QMutexLocker locker(&mtx_);
    pedestrian_ = ped;
}

Pedestrian sharedDatapool::getPedestrian() const
{
    QMutexLocker locker(&mtx_);
    return pedestrian_;
}

void sharedDatapool::updatePedestrian()
{
    QMutexLocker locker(&mtx_);
    double yMin = -38.0;
    double yMax = 38.0;
    if (pedestrian_.movingRight) {
        pedestrian_.y += pedestrian_.speed * 0.1;
        if (pedestrian_.y > yMax) {
            pedestrian_.y = yMax;
            pedestrian_.movingRight = false;
        }
    } else {
        pedestrian_.y -= pedestrian_.speed * 0.1;
        if (pedestrian_.y < yMin) {
            pedestrian_.y = yMin;
            pedestrian_.movingRight = true;
        }
    }
    pedestrian_.x = 110.0;
}

//停车位接口实现
ParkingSlot sharedDatapool::getParkingSlot() const
{
    QMutexLocker locker(&mtx_);
    return parkingSlot_;
}

void sharedDatapool::setParkingSlotOccupied(bool occupied)
{
    QMutexLocker locker(&mtx_);
    parkingSlot_.occupied = occupied;
}