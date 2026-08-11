#ifndef PLANNINGTHREAD_H
#define PLANNINGTHREAD_H

#include <thread>
#include <atomic>
#include <QObject>
#include "shareddatapool.h"

class PlanningThread : public QObject
{
    Q_OBJECT

public:
    explicit PlanningThread(sharedDatapool &pool, int vehicleId, QObject *parent = nullptr);
    ~PlanningThread();

    void start();
    void stop();
    void wait();

signals:
    void finished();

private:
    void run();

    sharedDatapool &pool_;
    int vehicleId_;
    std::thread thread_;
    std::atomic<bool> running_{true};
    bool parkingActive_ = false;
    bool passedTrafficLight_ = false;
    bool wasAvoiding_ = false;
    bool needResetYawAtLight_ = false;   // 新增：标记是否需要过红绿灯时归零
};

#endif // PLANNINGTHREAD_H