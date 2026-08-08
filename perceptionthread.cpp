#include "perceptionthread.h"
#include<QDebug>
#include<cstdlib>
PerceptionThread::PerceptionThread(sharedDatapool &pool, QObject *parent)
    : QThread(parent), pool_(pool)
{
}

void PerceptionThread::run()
{
    qDebug()<<"[感知]启动";

    Trafficlight tl = pool_.getTrafficLight();
    qDebug() << "[感知] 红绿灯初始状态:" << tl.getStateName()
             << " 位置:(" << tl.x << ", " << tl.y << ")"
             << " switchInterval:" << tl.switchInterval;

    int obs_id = 0;
    int counter = 0;
    QList<Obstacle> current = pool_.getObstacles();
    int count = current.size();

    while(pool_.isRunning())
    {
        counter++;

       // Trafficlight currentTl = pool_.getTrafficLight();
        //currentTl.update();
        //pool_.setTrafficLingt(currentTl);

        //pool_.updateTrafficLight();
        QList<Obstacle> currentObs = pool_.getObstacles();

        if(currentObs.size() < 1 )
        {
            obs_id++;
            VehicleState state = pool_.get();

            Obstacle obs;
            obs.id = obs_id;

            double farDistance = 10.0 + (rand() % 300) / 10.0;
            double laterlOffset = (rand() % 80 - 40) / 10.0;

            obs.x = state.x + farDistance;
            obs.y = state.y + laterlOffset;
            obs.speed = 0.0;

            pool_.addObstacle(obs);
            qDebug()<<"[感知]障碍物ID："<<obs_id
                     <<"距离："<<farDistance <<"m"
                     <<"速度"<<obs.speed<<"m/s"
                     <<" 总个数："<<pool_.getObstacles().size();
        }
        if(counter %10 == 0)
        {
            pool_.cleanObstacle();
        }
        if(counter %30 == 0)
        {
            Trafficlight currenTl = pool_.getTrafficLight();
            qDebug() <<"[感知]红绿灯状态:" <<currenTl.getStateName();
        }

        for(int i = 0;i<8 && pool_.isRunning();i++)
        {
            msleep(30);
        }


    }

    qDebug()<<"[感知]退出";
}