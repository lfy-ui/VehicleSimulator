#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsView>
#include "shareddatapool.h"
#include "localizationthread.h"
#include "perceptionthread.h"
#include "planningthread.h"
#include "monitorthread.h"
#include "simulatorview.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartClicked();
    void onPauseClicked();
    void onResetClicked();
    void onUpdateTimer();

private:
 sharedDatapool *pool_ = nullptr;
    SimulatorView *simulatorView_ = nullptr;
    QTimer *updateTimer_ = nullptr;

    // 线程指针
    LocalizationThread *locThread1_ = nullptr;
    PerceptionThread *perThread1_ = nullptr;
    PlanningThread *planThread1_ = nullptr;

    LocalizationThread *locThread2_ = nullptr;
    PerceptionThread *perThread2_ = nullptr;
    PlanningThread *planThread2_ = nullptr;

    MonitorThread *monThread_ = nullptr;

    // UI 控件
    QPushButton *startBtn_ = nullptr;
    QPushButton *pauseBtn_ = nullptr;
    QPushButton *resetBtn_ = nullptr;
    QLabel *statusLabel_ = nullptr;

    // 车辆1 状态
    QLabel *speedLabel_ = nullptr;
    QLabel *yawLabel_ = nullptr;
    QLabel *decisionLabel_ = nullptr;

    // 车辆2 状态  ✅ 新增
    QLabel *speedLabel2_ = nullptr;
    QLabel *yawLabel2_ = nullptr;
    QLabel *decisionLabel2_ = nullptr;

    QLabel *obstacleLabel_ = nullptr;
    QTextEdit *logTextEdit_ = nullptr;
    QLabel *stepLabel_ = nullptr;

    bool isRunning_ = false;

    void addLog(const QString &msg);
    void setupUI();
    //void setupUI();
};

#endif // MAINWINDOW_H