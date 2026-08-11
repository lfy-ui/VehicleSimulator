#include "perceptionthread.h"
#include <QDebug>
#include <chrono>
#include <cstdlib>

PerceptionThread::PerceptionThread(sharedDatapool &pool, int vehicleId, QObject *parent)
    : QObject(parent), pool_(pool), vehicleId_(vehicleId)
{
}

PerceptionThread::~PerceptionThread()
{
    stop();
    wait();
}

void PerceptionThread::start()
{
    running_ = true;
    thread_ = std::thread(&PerceptionThread::run, this);
}

void PerceptionThread::stop()
{
    running_ = false;
}

void PerceptionThread::wait()
{
    if (thread_.joinable()) {
        thread_.join();
    }
}

void PerceptionThread::run()
{
    qDebug() << "[感知] 车辆" << vehicleId_ << " std::thread 启动（障碍物在30m处）";

    bool obstacleGenerated = false;
    bool obstaclePassed = false;
    double obstacleX = 0.0;
    // ============================================================
    // 【修改】障碍物生成在车辆前方 28~32 米（约30米）
    // ============================================================
    const double OBS_DIST_MIN =20.0;
    const double OBS_DIST_MAX =25.0;

    while (running_) {
        VehicleState state = pool_.getVehicleState(vehicleId_);
        if (state.id != vehicleId_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        bool shouldGenerate = false;
        bool passedTrafficLight = (vehicleId_ == 1 && state.x > 110.0);

        if (!passedTrafficLight && !obstaclePassed) {
            if (!obstacleGenerated) {
                shouldGenerate = true;
            } else {
                if (state.x > obstacleX + 2.0) {  // 车身越过障碍物+2米视为驶过
                    obstaclePassed = true;
                    qDebug() << "[感知] 车辆" << vehicleId_ << " 已驶过障碍物，不再生成";
                }
            }
        }

        if (shouldGenerate) {
            double dist = OBS_DIST_MIN + (rand() % (int)((OBS_DIST_MAX - OBS_DIST_MIN) * 10)) / 10.0;
            double x = state.x + dist;
            double y = (vehicleId_ == 1) ? -20.0 : 20.0;

            // 先清理旧障碍物再生成新的
            pool_.cleanObstacle(vehicleId_);

            Obstacle obs;
            obs.id = 1;
            obs.vehicleId = vehicleId_;
            obs.x = x;
            obs.y = y;
            obs.speed = 0.0;

            pool_.addObstacle(obs);
            obstacleGenerated = true;
            obstacleX = x;

            qDebug() << "[感知] 车辆" << vehicleId_ << " 生成中心线障碍物 位置:(" << x << "," << y << ") 距离:" << dist;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    qDebug() << "[感知] 车辆" << vehicleId_ << " std::thread 退出";
    emit finished();
}