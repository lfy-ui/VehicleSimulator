#include "localizationthread.h"
#include<QDebug>
#include<cmath>
LocalizationThread::LocalizationThread(sharedDatapool &pool, QObject *parent)
    : QThread{parent},pool_(pool)
{}

void LocalizationThread::run()
{
    qDebug()<<"[定位]启动";
    VehicleState state;
    int cycle = 0;
    double currentSpeed = 0.0;
    double currentSpeeding = 0.0;

    const double MAX_ACCEL = 0.15;   // 最大加速度 (m/s²)
    const double MAX_DECEL = 0.25;   // 最大减速度 (m/s²)
    const double MAX_SPEED = 10.0;   // 最大速度限制
    const double MIN_SPEED = 0.0;
    int returnTimer = 0;
    bool isReturning = false;

    while(pool_.isRunning()){

        ControlCommand cmd = pool_.getControlCommand();

        if(currentSpeed < cmd.targetSpeed)
        {
            currentSpeed += MAX_ACCEL;
        }else if(currentSpeed > cmd.targetSpeed)
        {
            currentSpeed -= MAX_DECEL;
        }
        currentSpeed = qMax(MIN_SPEED,qMin(MAX_SPEED,currentSpeed));

        if (cmd.targetSteering == 0.0)
        {
            // 快速回正
            if (currentSpeeding > 0.05) {
                currentSpeeding -= 0.15;
                isReturning = true;
                returnTimer++;
            } else if (currentSpeeding < -0.1) {
                currentSpeeding += 0.2;
                isReturning = true;
                returnTimer++;
            } else {
                currentSpeeding = 0.0;
                isReturning = false;
                returnTimer = 0;
            }
            if(returnTimer > 20)
            {
                currentSpeeding = 0.0;
                isReturning = false;
                returnTimer = 0;
            }
        } else {
            // 正常转向
            if (currentSpeeding < cmd.targetSteering) {
                currentSpeeding += 0.25;
            } else if (currentSpeeding > cmd.targetSteering) {
                currentSpeeding -= 0.25;
            }
        }
        currentSpeeding = qMax(-1.0, qMin(1.0, currentSpeeding));
            static int returnTimer = 0;
        if (cmd.targetSteering == 0.0 && fabs(currentSpeeding) > 0.01) {
            returnTimer++;
            if (returnTimer > 30) {
                currentSpeeding = 0.0;
                returnTimer = 0;
                qDebug() << "[定位] 回正超时，强制归零";
            }
        } else {
            returnTimer = 0;
        }
        if (state.y > 25.0) {
            state.y = 25.0;
            currentSpeeding = -0.3;  // 强制转向回道路
        } else if (state.y < -25.0) {
            state.y = -25.0;
            currentSpeeding = 0.3;   // 强制转向回道路
        }

        state.x += currentSpeed *cos(state.yaw) *0.1;
        state.y += currentSpeed *sin(state.yaw) *0.1;
        state.yaw += currentSpeeding * 0.12;
        state.speed += currentSpeed;

        pool_.update(state);
        qDebug()<<"[定位]更新"<<++cycle
                 <<"速度："<<currentSpeed
                 <<"转向："<<currentSpeeding
                 <<"目标速度"<<cmd.targetSpeed;

        for(int i = 0;i < 8 && pool_.isRunning();i++)
        {
            msleep(10);
        }
    }
    qDebug()<<"[定位]退出";
}
