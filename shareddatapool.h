#ifndef SHAREDDATAPOOL_H
#define SHAREDDATAPOOL_H

#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QList>
#include <QString>
#include <QDebug>
#include <QColor>

struct VehicleState{
    double x = 0.0,y = 0.0,speed = 0.0,yaw = 0.0;

    QString toString() const{
        return QString("位置（%1，%2） 速度：%3m/s")
            .arg(x,0,'f',2)
            .arg(y,0,'f',2)
            .arg(speed,0,'f',2);
    }
};

enum class TrafficlightState
{
    Red,
    Yellow,
    Green
};

struct Trafficlight
{
    int id = 0;
    double x = 0;
    double y = 0;
    TrafficlightState state = TrafficlightState::Green;
    int timer = 0;
    int switchInterval = 200;

    QString getStateName() const
    {
        switch(state)
        {
        case TrafficlightState::Green: return "绿灯";
        case TrafficlightState::Red: return "红灯";
        case TrafficlightState::Yellow: return "黄灯";
        }
        return "未知";
    }

    QColor getColor() const
    {
        switch(state)
        {
        case TrafficlightState::Red: return Qt::red;
        case TrafficlightState::Green: return Qt::green;
        case TrafficlightState::Yellow: return Qt::yellow;
        }
        return Qt::gray;
    }

    void update()
    {
        timer++;
        if(timer >=switchInterval)
        {
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

struct Obstacle{
    int id = 0;
    double x = 0.0,y = 0.0,speed = 0.0;
};

struct ControlCommand
{
    double targetSpeed = 5.0;
    double targetSteering = 0.0;
    QString reason = "正常行驶";
};

class sharedDatapool
{
public:
    sharedDatapool();

    void update(const VehicleState &s);
    VehicleState get();

    void addObstacle(const Obstacle &obs);
    QList<Obstacle> getObstacles() const;

    void stopSystem();
    bool isRunning() const;

    void setControlCommand(const ControlCommand &cmd);
    ControlCommand getControlCommand() const;

    void cleanObstacle();

    void setTrafficLingt(const Trafficlight &tl);
    Trafficlight getTrafficLight() const;
    void updateTrafficLight();

private:
    VehicleState state_;
    bool hasNewDate = false;
    bool running = true;
    QList<Obstacle> obstacles_;
    QQueue<int> sen_;
    Trafficlight trafficLight_;

    ControlCommand cmd_;

    mutable QMutex mtx_;
    QWaitCondition cv_;
};

#endif // SHAREDDATAPOOL_H
