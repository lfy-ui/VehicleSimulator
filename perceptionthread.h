#ifndef PERCEPTIONTHREAD_H
#define PERCEPTIONTHREAD_H

#include <thread>
#include <atomic>
#include <QObject>
#include "shareddatapool.h"

class PerceptionThread : public QObject
{
    Q_OBJECT

public:
    explicit PerceptionThread(sharedDatapool &pool, int vehicleId, QObject *parent = nullptr);
    ~PerceptionThread();

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
};

#endif // PERCEPTIONTHREAD_H