#include "localizationthread.h"
#include <QDebug>
#include <cmath>

LocalizationThread::LocalizationThread(sharedDatapool &pool, QObject *parent)
    : QThread{parent}, pool_(pool)
{
}

void LocalizationThread::run()
{
    qDebug() << "[定位]启动";
    VehicleState state;
    int cycle = 0;
    double currentSpeed = 0.0;
    double currentSpeeding = 0.0;

    const double MAX_ACCEL = 0.15;
    const double MAX_DECEL = 0.25;
    const double MAX_SPEED = 10.0;
    const double MIN_SPEED = 0.0;

    int returnTimer = 0;

    while (pool_.isRunning()) {
        ControlCommand cmd = pool_.getControlCommand();

        //  速度控制
        if (currentSpeed < cmd.targetSpeed) {
            currentSpeed += MAX_ACCEL;
            if (currentSpeed > cmd.targetSpeed) currentSpeed = cmd.targetSpeed;
        } else if (currentSpeed > cmd.targetSpeed) {
            currentSpeed -= MAX_DECEL;
            if (currentSpeed < cmd.targetSpeed) currentSpeed = cmd.targetSpeed;
        }
        currentSpeed = qMax(MIN_SPEED, qMin(MAX_SPEED, currentSpeed));

        //  转向控制

        if (cmd.targetSteering != 0.0) {
            returnTimer = 0;
            if (currentSpeeding < cmd.targetSteering) {
                currentSpeeding += 0.25;
                if (currentSpeeding > cmd.targetSteering) currentSpeeding = cmd.targetSteering;
            } else if (currentSpeeding > cmd.targetSteering) {
                currentSpeeding -= 0.25;
                if (currentSpeeding < cmd.targetSteering) currentSpeeding = cmd.targetSteering;
            }
        } else {

            if (fabs(currentSpeeding) < 0.01) {
                currentSpeeding = 0.0;
                returnTimer = 0;
            } else {
                // 回正
                if (currentSpeeding > 0) {
                    currentSpeeding -= 0.15;
                    if (currentSpeeding < 0) currentSpeeding = 0;
                } else {
                    currentSpeeding += 0.15;
                    if (currentSpeeding > 0) currentSpeeding = 0;
                }

                returnTimer++;
                qDebug() << "[定位] 回正中... timer:" << returnTimer << " steering:" << currentSpeeding;

                // 超时强制归零
                if (returnTimer > 20) {
                    currentSpeeding = 0.0;
                    returnTimer = 0;
                    qDebug() << "[定位] ★★★ 强制回正!";
                }
            }
        }

        currentSpeeding = qMax(-1.0, qMin(1.0, currentSpeeding));

        // 边界约束
        const double ROAD_LIMIT = 10.0;
        if (state.y > ROAD_LIMIT) {
            state.y = ROAD_LIMIT;

            if (currentSpeeding > -0.1) currentSpeeding -= 0.05;
        } else if (state.y < -ROAD_LIMIT) {
            state.y = -ROAD_LIMIT;
            if (currentSpeeding < 0.1) currentSpeeding += 0.05;
        }

        //物理更新
        state.x += currentSpeed * cos(state.yaw) * 0.1;
        state.y += currentSpeed * sin(state.yaw) * 0.1;
        state.yaw += currentSpeeding * 0.12;
        state.speed = currentSpeed;

        pool_.update(state);

        // 调试日志（每10帧）
        if (cycle % 10 == 0) {
            qDebug() << "[定位] 速度:" << currentSpeed
                     << " 转向:" << currentSpeeding
                     << " 目标转向:" << cmd.targetSteering
                     << " y:" << state.y;
        }

        cycle++;

        for (int i = 0; i < 8 && pool_.isRunning(); i++) {
            msleep(10);
        }
    }

    qDebug() << "[定位]退出";
}
