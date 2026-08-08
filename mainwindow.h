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

    LocalizationThread *locThread_ = nullptr;
    PerceptionThread *perThread_ = nullptr;
    PlanningTherad *planThread_ = nullptr;
    MonitorThread *monThread_ = nullptr;

    QPushButton *startBtn_ = nullptr;
    QPushButton *pauseBtn_ = nullptr;
    QPushButton *resetBtn_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QLabel *speedLabel_ = nullptr;
    QLabel *yawLabel_ = nullptr;
    QLabel *obstacleLabel_ = nullptr;
    QLabel *decisionLabel_ = nullptr;
    QTextEdit *logTextEdit_ = nullptr;
    QLabel *stepLabel_ = nullptr;

    bool isRunning_ = false;

    void addLog(const QString &msg);
    void setupUI();
};

#endif // MAINWINDOW_H