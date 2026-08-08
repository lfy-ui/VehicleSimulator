#ifndef MONITORTHREAD_H
#define MONITORTHREAD_H

#include <QObject>
#include<QThread>
#include"shareddatapool.h"
class MonitorThread : public QThread
{
    Q_OBJECT
public:
    explicit MonitorThread(sharedDatapool &pool, QObject *parent = nullptr);

protected:
    void run() override;

private:
    sharedDatapool &pool_;
};

#endif // MONITORTHREAD_H
