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


        QList<Obstacle> currentObs = pool_.getObstacles();

        if(currentObs.size() < 1 )//限制障碍物1个
        {
            obs_id++;
            VehicleState state = pool_.get();

            Obstacle obs;
            obs.id = obs_id;

            double farDistance = 15.0 + (rand() % 300) / 10.0;//限制生成距离x
            double laterlOffset = (rand() % 80 - 40) / 10.0;//限制y
            //障碍物坐标x，y
            obs.x = state.x + farDistance;
            obs.y = state.y + laterlOffset;
            obs.speed = 0.0;

            pool_.addObstacle(obs);
            qDebug()<<"[感知]障碍物ID："<<obs_id
                     <<"距离："<<farDistance <<"m"
                     <<"速度"<<obs.speed<<"m/s"
                     <<" 总个数："<<pool_.getObstacles().size();
        }
        if(counter %10 == 0)//十帧清理障碍物
        {
            pool_.cleanObstacle();
        }
        if(counter %30 == 0)//三十帧感知红绿灯
        {
            Trafficlight currenTl = pool_.getTrafficLight();
            qDebug() <<"[感知]红绿灯状态:" <<currenTl.getStateName();
        }

        for(int i = 0;i<8 && pool_.isRunning();i++)
        {
            msleep(100);
        }


    }

    qDebug()<<"[感知]退出";
}