#include "mainwindow.h"

#include <QApplication>
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
     qDebug() << "开始创建主窗口...";
    MainWindow w;
    w.show();
    qDebug() << "窗口已显示，进入事件循环...";

    return a.exec();
}
