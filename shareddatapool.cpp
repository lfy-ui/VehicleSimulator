#include "shareddatapool.h"
#include<cmath>
#include<QThread>

sharedDatapool::sharedDatapool() {
    running = true;
    hasNewDate = false;

    trafficLight_.id = 1;
    trafficLight_.x = 100.0;
    trafficLight_.y = -30.0;
    trafficLight_.state = TrafficlightState::Green;
    trafficLight_.switchInterval = 30;
    trafficLight_.timer = 0;
}

void sharedDatapool::update(const VehicleState &s)
{
    QMutexLocker locker(&mtx_);
    state_ = s;
    hasNewDate = true;
    sen_.enqueue(1);
    qDebug()<<"[写入]"<<state_.toString();
    cv_.wakeOne();
}

VehicleState sharedDatapool::get()
{
    QMutexLocker locker(&mtx_);

    while(!hasNewDate && running){
        cv_.wait(&mtx_,100);
    }
    if(!running){
        return VehicleState();
    }
    hasNewDate = false;
    qDebug()<<"[读取]"<<state_.toString();
    return state_;
}

void sharedDatapool::addObstacle(const Obstacle &obs)
{
    QMutexLocker locker(&mtx_);
    double carX = state_.x;
    double carY = state_.y;

    QList<Obstacle> remaining;
    for(const Obstacle& o : obstacles_)
    {
        double dx = o.x - carX;
        //double dy = o.y - carY;
        double dist = sqrt(dx * dx );

        if(dist < 20.0 && o.x > carX - 2.0)
        {
            remaining.append(o);
        }
    }

    obstacles_ = remaining;

    if(obstacles_.size() < 5)
    {
        obstacles_.append(obs);
        qDebug()<<"[数据池]当前障碍物数量："<<obstacles_.size();
    }

}

QList<Obstacle> sharedDatapool::getObstacles() const
{
    QMutexLocker locker(&mtx_);
    return obstacles_;
}

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

void sharedDatapool::setControlCommand(const ControlCommand &cmd)
{
    QMutexLocker locker(&mtx_);
    cmd_ = cmd;
}

ControlCommand sharedDatapool::getControlCommand() const
{
    QMutexLocker locker(&mtx_);
    return cmd_;
}

void sharedDatapool::cleanObstacle()
{
    QMutexLocker locker(&mtx_);
    double carX = state_.x;

    QList<Obstacle> reaining;
    for(const Obstacle& o: obstacles_)
    {
        double dx = o.x -carX;
        if(dx > 0 && dx < 20.0)
        {
            reaining.append(o);
        }
    }
    obstacles_ = reaining;
}

void sharedDatapool::setTrafficLingt(const Trafficlight& tl)
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
