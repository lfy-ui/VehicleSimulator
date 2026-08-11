#ifndef LOCALIZATIONTHREAD_H
#define LOCALIZATIONTHREAD_H

#include <thread>
#include <atomic>
#include <QObject>
#include "shareddatapool.h"

class LocalizationThread : public QObject
{
    Q_OBJECT

public:
    explicit LocalizationThread(sharedDatapool &pool, int vehicleId, QObject *parent = nullptr);
    ~LocalizationThread();

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

#endif // LOCALIZATIONTHREAD_H