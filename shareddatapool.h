#ifndef SHAREDDATAPOOL_H
#define SHAREDDATAPOOL_H

#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QList>
#include <QString>
#include <QDebug>
#include <QColor>

struct VehicleState {
    double x = 0.0, y = 0.0, speed = 0.0, yaw = 0.0;
    int id = 1;

    QString toString() const {
        return QString("车辆%1 位置(%2, %3) 速度:%4m/s")
            .arg(id)
            .arg(x, 0, 'f', 2)
            .arg(y, 0, 'f', 2)
            .arg(speed, 0, 'f', 2);
    }
};

enum class TrafficlightState {
    Red,
    Yellow,
    Green
};

struct Trafficlight {
    int id = 0;
    double x = 0;
    double y = 0;
    TrafficlightState state = TrafficlightState::Green;
    int timer = 0;
    int switchInterval = 60;

    QString getStateName() const {
        switch (state) {
        case TrafficlightState::Green: return "绿灯";
        case TrafficlightState::Red: return "红灯";
        case TrafficlightState::Yellow: return "黄灯";
        }
        return "未知";
    }

    QColor getColor() const {
        switch (state) {
        case TrafficlightState::Red: return Qt::red;
        case TrafficlightState::Green: return Qt::green;
        case TrafficlightState::Yellow: return Qt::yellow;
        }
        return Qt::gray;
    }

    void update() {
        timer++;
        if (timer >= switchInterval) {
            timer = 0;
            switch (state) {
            case TrafficlightState::Red:
                state = TrafficlightState::Green;
                break;
            case TrafficlightState::Green:
                state = TrafficlightState::Yellow;
                break;
            case TrafficlightState::Yellow:
                state = TrafficlightState::Red;
                break;
            }
        }
    }
};

struct Obstacle {
    int id = 0;
    double x = 0.0, y = 0.0, speed = 0.0;
    int vehicleId = 1;
};

struct ControlCommand {
    int vehicleId = 1;
    double targetSpeed = 5.0;
    double targetSteering = 0.0;
    QString reason = "正常行驶";
    bool resetYaw = false;
};

struct Pedestrian {
    int id = 0;
    double x = 0.0;
    double y = 0.0;
    double speed = 0.8;
    bool movingRight = true;
};

// ============================================================
// 【新增】停车位结构
// ============================================================
struct ParkingSlot {
    int id = 1;
    double x = 0.0;
    double y = 0.0;
    double width = 20.0;
    double height = 8.0;
    bool occupied = false;
};

class sharedDatapool
{
public:
    sharedDatapool();

    void addVehicle(int id, double x, double y);
    void updateVehicleState(const VehicleState &s);
    VehicleState getVehicleState(int id) const;
    QList<VehicleState> getAllVehicleStates() const;

    void update(const VehicleState &s);
    VehicleState get();

    void addObstacle(const Obstacle &obs);
    QList<Obstacle> getObstacles() const;
    QList<Obstacle> getObstacles(int vehicleId) const;

    void stopSystem();
    bool isRunning() const;

    void setControlCommand(const ControlCommand &cmd);
    ControlCommand getControlCommand() const;
    ControlCommand getControlCommand(int vehicleId) const;

    void cleanObstacle();
    void cleanObstacle(int vehicleId);

    void setTrafficLight(const Trafficlight &tl);
    Trafficlight getTrafficLight() const;
    void updateTrafficLight();

    void setPedestrian(const Pedestrian &ped);
    Pedestrian getPedestrian() const;
    void updatePedestrian();

    // ============================================================
    // 【新增】停车位接口
    // ============================================================
    ParkingSlot getParkingSlot() const;
    void setParkingSlotOccupied(bool occupied);

private:
    QList<VehicleState> vehicles_;
    QList<ControlCommand> commands_;
    VehicleState state_;
    bool hasNewDate = false;
    bool running = true;
    QList<Obstacle> obstacles_;
    QQueue<int> sen_;
    Trafficlight trafficLight_;
    ControlCommand cmd_;
    Pedestrian pedestrian_;
    ParkingSlot parkingSlot_;   // 停车位

    mutable QMutex mtx_;
    QWaitCondition cv_;
};

#endif // SHAREDDATAPOOL_H