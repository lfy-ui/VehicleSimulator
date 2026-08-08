#include "planningthread.h"
#include<QDebug>
#include<cmath>
PlanningTherad::PlanningTherad(sharedDatapool &pool,QObject *parent)
    : QThread{parent},pool_(pool)
{}

void PlanningTherad::run()
{
    qDebug()<<"[规划]启动";
    bool isReturning = false;
    int returnCount = 0;

    while(pool_.isRunning())
    {
        VehicleState state = pool_.get();
        if(!pool_.isRunning())break;
        QList<Obstacle> obstacles = pool_.getObstacles();

        Trafficlight tl = pool_.getTrafficLight();

        double dx1 = tl.x - state.x;
        double dy1 = tl.y - state.y;
        double disTolight = sqrt(dx1 * dx1 +dy1 * dy1);

        qDebug() << "[规划] 红绿灯 距离:" << disTolight
                 << " 状态:" << tl.getStateName()
                 << " 车辆x:" << state.x << " 红绿灯x:" << tl.x;

        bool detected = false;
        double minDist = 100.0;
        double detectionRange = 50.0;
        double angleRange = 60.0 * 3.14159 / 180;

        for(const Obstacle& obs : obstacles){
            double dx = obs.x - state.x;
            double dy = obs.y - state.y;
            double dist = sqrt(dx * dx + dy * dy);
            if(dist > detectionRange)continue;

            double angle = atan2(dy,dx) - state.yaw;
            while(angle > M_PI)angle -= 2* M_PI;
            while(angle < -M_PI)angle += 2* M_PI;

            if(abs(angle) < angleRange)
            {
                detected = true;
                if(dist < minDist)
                {
                    minDist = dist;
                    //nearestObs = obs;
                }

            }
        }

        ControlCommand cmd;

        if(disTolight <80.0 && tl.state == TrafficlightState::Red)//检测红绿灯距离
        {
            cmd.targetSpeed = 0.0;
            cmd.targetSteering = 0.0;
            cmd.reason = "红灯停车(距离：" +QString::number(disTolight,'f',1) + "m)";
            isReturning = false;
        }
        else if(disTolight <85.0 &&tl.state == TrafficlightState::Yellow)
        {
            cmd.targetSpeed = qMin(3.0,state.speed * 0.5);
            cmd.targetSteering = 0.0;
            cmd.reason = "黄灯减速（距离："+QString::number(disTolight,'f',1) +"m";
            isReturning = false;
        }
        else if(detected)
        {
            bool isReturning = false;
            int returnCount = 0;
            //isAvoiding_ = false;
            if(minDist >20.0)
            {
                cmd.targetSpeed = qMin(8.0,state.speed * 0.8);
                cmd.targetSteering = 0.0;
                cmd.reason = "前方有障碍物！！！" + QString::number(minDist,'f',1);
                //isAvoiding_ = false;
            }else if(minDist < 2.0)
            {
                cmd.targetSpeed = 0.0;
                cmd.targetSteering = 0.0;
                cmd.reason = "停车！！距离" + QString::number(minDist,'f',1);
                //isAvoiding_ = false;
            }else
            {   //isAvoiding_ = true;
                bool canleft = true;
                bool canright = true;

                for(const Obstacle& obs : obstacles)
                {
                    double dx = obs.x - state.x;
                    double dy = obs.y - state.y;
                    double dist = sqrt(dx * dx + dy * dy);
                    if(dist > 10.0)continue;
                    double angle = atan2(dy,dx) - state.yaw;
                    while(angle > M_PI) angle -= 2*M_PI;
                    while(angle < -M_PI) angle += 2*M_PI;

                    if(angle > 0.15 && angle < 0.6)
                    {
                        canleft = false;
                    }
                    if(angle <-0.15 && angle >-0.6)
                    {
                        canright = false;
                    }
                }

                if(canleft && canright)
                {
                    if(rand() %2 == 0)
                    {
                        cmd.targetSteering = 0.6;
                        cmd.reason = "左转避让";
                    }else
                    {
                        cmd.targetSteering = -0.6;
                        cmd.reason = "右转避让";
                    }
                    cmd.targetSpeed = qMin(8.0,state.speed);

                }else if(canleft)
                {
                    cmd.targetSteering = 0.6;
                    cmd.targetSpeed = qMin(8.0,state.speed);
                    cmd.reason = "左转避让";
                }else if(canright)
                {
                    cmd.targetSteering = -0.6;
                    cmd.targetSpeed = qMin(8.0,state.speed);
                    cmd.reason = "右转避让";
                }else
                {
                    cmd.targetSpeed = qMax(0.0,state.speed - 2.0);
                    cmd.targetSteering = 0.0;
                    cmd.reason = "两侧受阻，减速等待";
                }
                cmd.targetSpeed = qMin(6.0,qMax(3.0,state.speed * 0.8));

            }
        }else
        {
            if (isReturning) {
                returnCount++;
                cmd.targetSpeed = qMin(8.0, state.speed + 0.3);
                cmd.targetSteering = 0.0;
                cmd.reason = "↩️ 回正方向 (" + QString::number(returnCount) + ")";

                if (returnCount > 5) {
                    isReturning = false;
                    cmd.reason = "✅ 回正完成";
                }
            } else {
                cmd.targetSpeed = qMin(8.0, state.speed + 0.3);
                cmd.targetSteering = 0.0;
                cmd.reason = "✅ 正常行驶";
            }
        }
        if (!detected && cmd.targetSteering == 0.0 && state.yaw != 0.0) {
            isReturning = true;
            returnCount = 0;
            qDebug() << "[规划] 启动回正";
        }
        pool_.setControlCommand(cmd);
        qDebug()<<"[规划]"<<cmd.reason;

        for(int i = 0;i <6 && pool_.isRunning();i++)
        {
            msleep(5);
        }
    }

    qDebug() <<"[规划]退出";
}
