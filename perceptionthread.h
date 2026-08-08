#ifndef PERCEPTIONTHREAD_H
#define PERCEPTIONTHREAD_H

#include <QObject>
#include <QThread>
#include"shareddatapool.h"

class PerceptionThread : public QThread
{
    Q_OBJECT
public:
    explicit PerceptionThread(sharedDatapool &pool ,QObject *parent = nullptr);

protected:
    void run() override;

private:
    sharedDatapool &pool_;
};

#endif // PERCEPTIONTHREAD_H
