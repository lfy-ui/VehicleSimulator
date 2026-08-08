#include "monitorthread.h"
#include<QDebug>
MonitorThread::MonitorThread(sharedDatapool &pool,QObject *parent)
    :QThread(parent),pool_(pool)
{}

void MonitorThread::run()
{
    qDebug()<<"[监控]启动]";

    while(pool_.isRunning()){
        VehicleState state = pool_.get();
        if (!pool_.isRunning()) break;


        qDebug() << "\n========== 系统状态 ==========";
        qDebug() << "车辆:" << state.toString();
        qDebug() << "================================\n";

        msleep(1000);
    }
    qDebug()<<"[监控]退出";
}