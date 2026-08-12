#include "mainwindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QGroupBox>
#include <QGridLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
     qDebug() << "[MainWindow] 开始构造";
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    qDebug() << "[MainWindow] centralWidget 创建";

    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    qDebug() << "[MainWindow] mainLayout 创建";

    // ===== 左侧布局 =====
    QVBoxLayout* leftLayout = new QVBoxLayout();
     qDebug() << "[MainWindow] leftLayout 创建";

    simulatorView_ = new SimulatorView(this);
     qDebug() << "[MainWindow] SimulatorView 创建";
    leftLayout->addWidget(simulatorView_);
    qDebug() << "[MainWindow] SimulatorView 添加";

    QHBoxLayout* controlLayout = new QHBoxLayout();
    QPushButton* startBtn = new QPushButton("启动", this);
    QPushButton* pauseBtn = new QPushButton("暂停", this);
    QPushButton* resetBtn = new QPushButton("重置", this);
    QLabel* stepLabel = new QLabel("步长: 50ms", this);

    controlLayout->addWidget(startBtn);
    controlLayout->addWidget(pauseBtn);
    controlLayout->addWidget(resetBtn);
    controlLayout->addWidget(stepLabel);
    leftLayout->addLayout(controlLayout);
    mainLayout->addLayout(leftLayout, 3);

    // ===== 右侧布局 =====
    QWidget* rightWidget = new QWidget(this);
    rightWidget->setMaximumWidth(320);
    rightWidget->setMinimumWidth(250);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setSpacing(10);

    QGroupBox* group1 = new QGroupBox("🚗 车辆1 状态", this);
    QGridLayout* layout1 = new QGridLayout(group1);
    QLabel* statusLabel = new QLabel("就绪", this);
    QLabel* speedLabel = new QLabel("速度: 0.0 m/s", this);
    QLabel* yawLabel = new QLabel("航向: 0.00 rad", this);
    QLabel* decisionLabel = new QLabel("决策: 正常行驶", this);
    layout1->addWidget(statusLabel, 0, 0);
    layout1->addWidget(speedLabel, 1, 0);
    layout1->addWidget(yawLabel, 2, 0);
    layout1->addWidget(decisionLabel, 3, 0);
    rightLayout->addWidget(group1);

    QGroupBox* group2 = new QGroupBox("🚗 车辆2 状态", this);
    QGridLayout* layout2 = new QGridLayout(group2);
    QLabel* statusLabel2 = new QLabel("就绪", this);
    QLabel* speedLabel2 = new QLabel("速度: 0.0 m/s", this);
    QLabel* yawLabel2 = new QLabel("航向: 0.00 rad", this);
    QLabel* decisionLabel2 = new QLabel("决策: 正常行驶", this);
    layout2->addWidget(statusLabel2, 0, 0);
    layout2->addWidget(speedLabel2, 1, 0);
    layout2->addWidget(yawLabel2, 2, 0);
    layout2->addWidget(decisionLabel2, 3, 0);
    rightLayout->addWidget(group2);

    QLabel* obstacleLabel = new QLabel("🟨 障碍物: 0", this);
    rightLayout->addWidget(obstacleLabel);

    QGroupBox* logGroup = new QGroupBox("📋 系统日志", this);
    QVBoxLayout* logLayout = new QVBoxLayout(logGroup);
    QTextEdit* logTextEdit = new QTextEdit(this);
    logTextEdit->setReadOnly(true);
    logTextEdit->setMinimumHeight(150);
    logLayout->addWidget(logTextEdit);
    rightLayout->addWidget(logGroup);

    mainLayout->addWidget(rightWidget);

    // ===== 保存控件指针 =====
    startBtn_ = startBtn;
    pauseBtn_ = pauseBtn;
    resetBtn_ = resetBtn;
    statusLabel_ = statusLabel;
    speedLabel_ = speedLabel;
    yawLabel_ = yawLabel;
    decisionLabel_ = decisionLabel;
    speedLabel2_ = speedLabel2;
    yawLabel2_ = yawLabel2;
    decisionLabel2_ = decisionLabel2;
    obstacleLabel_ = obstacleLabel;
    logTextEdit_ = logTextEdit;
    stepLabel_ = stepLabel;

    connect(startBtn, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(pauseBtn, &QPushButton::clicked, this, &MainWindow::onPauseClicked);
    connect(resetBtn, &QPushButton::clicked, this, &MainWindow::onResetClicked);

    pauseBtn->setEnabled(false);
    resetBtn->setEnabled(false);

    pool_ = new sharedDatapool();
    qDebug() << "[MainWindow] sharedDatapool 创建";

    updateTimer_ = new QTimer(this);
    qDebug() << "[MainWindow] 定时器创建";
    connect(updateTimer_, &QTimer::timeout, this, &MainWindow::onUpdateTimer);
     qDebug() << "[MainWindow] 信号连接完成";

    addLog("[INFO] 仿真平台已启动");
     qDebug() << "[MainWindow] 构造函数完成";
}

MainWindow::~MainWindow()
{
    if (updateTimer_) {
        updateTimer_->stop();
    }
    if (pool_) {
        pool_->stopSystem();
    }

    if (locThread1_) { locThread1_->wait(); delete locThread1_; locThread1_ = nullptr; }
    if (perThread1_) { perThread1_->wait(); delete perThread1_; perThread1_ = nullptr; }
    if (planThread1_) { planThread1_->wait(); delete planThread1_; planThread1_ = nullptr; }

    if (locThread2_) { locThread2_->wait(); delete locThread2_; locThread2_ = nullptr; }
    if (perThread2_) { perThread2_->wait(); delete perThread2_; perThread2_ = nullptr; }
    if (planThread2_) { planThread2_->wait(); delete planThread2_; planThread2_ = nullptr; }

    if (monThread_) { monThread_->wait(); delete monThread_; monThread_ = nullptr; }

    if (pool_) {
        delete pool_;
        pool_ = nullptr;
    }
}

void MainWindow::onStartClicked()
{
    if (isRunning_) return;

    qDebug() << "[MainWindow] 点击启动";

    isRunning_ = true;
    startBtn_->setEnabled(false);
    pauseBtn_->setEnabled(true);
    resetBtn_->setEnabled(false);
    statusLabel_->setText("运行中");

    delete pool_;
    pool_ = new sharedDatapool();

    locThread1_ = new LocalizationThread(*pool_, 1, this);
    perThread1_ = new PerceptionThread(*pool_, 1, this);
    planThread1_ = new PlanningThread(*pool_, 1, this);

    locThread2_ = new LocalizationThread(*pool_, 2, this);
    perThread2_ = new PerceptionThread(*pool_, 2, this);
    planThread2_ = new PlanningThread(*pool_, 2, this);

    monThread_ = new MonitorThread(*pool_, this);

    locThread1_->start();
    perThread1_->start();
    planThread1_->start();

    locThread2_->start();
    perThread2_->start();
    planThread2_->start();

    monThread_->start();

    QThread::msleep(100);

    updateTimer_->start(50);

    addLog("[INFO] ✅ 双车系统启动");
}

void MainWindow::onPauseClicked()
{
    if (!isRunning_) return;

    isRunning_ = false;

    startBtn_->setEnabled(true);
    pauseBtn_->setEnabled(false);
    resetBtn_->setEnabled(true);
    statusLabel_->setText("已暂停");

    if (updateTimer_) {updateTimer_->stop();}
    if (pool_) {pool_->stopSystem();}

    if (locThread1_) {locThread1_->stop();locThread1_->wait();delete locThread1_;locThread1_ = nullptr;}
    if (perThread1_) {perThread1_->stop();perThread1_->wait();delete perThread1_;perThread1_ = nullptr;}
    if (planThread1_) {planThread1_->stop();planThread1_->wait();delete planThread1_;planThread1_ = nullptr;}

    if (locThread2_) {locThread2_->stop();locThread2_->wait();delete locThread2_;locThread2_ = nullptr;}
    if (perThread2_) {perThread2_->stop();perThread2_->wait();delete perThread2_;perThread2_ = nullptr;}
    if (planThread2_) {planThread2_->stop();planThread2_->wait();delete planThread2_;planThread2_ = nullptr;}

    if (monThread_) {monThread_->wait();delete monThread_;monThread_ = nullptr;}

    if (simulatorView_) {simulatorView_->resetView();}

    addLog("[INFO] ⏸ 双车系统暂停");
}

void MainWindow::onResetClicked()
{
    if (isRunning_) {
        onPauseClicked();
    }

    delete pool_;
    pool_ = new sharedDatapool();

    simulatorView_->resetView();

    startBtn_->setEnabled(true);
    pauseBtn_->setEnabled(false);
    resetBtn_->setEnabled(false);
    statusLabel_->setText("已重置");
    addLog("[INFO] 场景已重置");
}

void MainWindow::onUpdateTimer()
{
    // 更新红绿灯和行人
    pool_->updateTrafficLight();
    pool_->updatePedestrian();

    QList<VehicleState> vehicles = pool_->getAllVehicleStates();
    QList<Obstacle> allObstacles = pool_->getObstacles();
    Trafficlight tl = pool_->getTrafficLight();
    Pedestrian ped = pool_->getPedestrian();

    VehicleState state1 = pool_->getVehicleState(1);
    VehicleState state2 = pool_->getVehicleState(2);

    QList<QPointF> trajectory;
    for (int i = 0; i < 20; i++) {
        trajectory.append(QPointF(state1.x + i * 0.5, state1.y + sin(i * 0.2) * 0.5));
    }

    // 将行人传递给画面
    simulatorView_->updateScene(vehicles, allObstacles, trajectory, tl, ped);

    // 车辆1信息
    speedLabel_->setText("速度: " + QString::number(state1.speed, 'f', 1) + " m/s");
    yawLabel_->setText("航向: " + QString::number(state1.yaw, 'f', 2) + " rad");
    ControlCommand cmd1 = pool_->getControlCommand(1);
    decisionLabel_->setText("决策: " + cmd1.reason);

    // 车辆2信息
    speedLabel2_->setText("速度: " + QString::number(state2.speed, 'f', 1) + " m/s");
    yawLabel2_->setText("航向: " + QString::number(state2.yaw, 'f', 2) + " rad");
    ControlCommand cmd2 = pool_->getControlCommand(2);
    decisionLabel2_->setText("决策: " + cmd2.reason);

    obstacleLabel_->setText("🟨 障碍物: " + QString::number(allObstacles.size()));

    stepLabel_->setText("⏱ 步长: 50ms");

    // 近距离障碍物警告（车辆1）
    for (const auto& obs : allObstacles) {
        double dx = obs.x - state1.x;
        double dy = obs.y - state1.y;
        if (sqrt(dx*dx + dy*dy) < 5.0) {
            addLog("[WARN] ⚠️ 车辆1 检测到近距离障碍物!");
            break;
        }
    }

    static int printCounter = 0;
    printCounter++;
    if (printCounter % 30 == 0) {
        qDebug() << "[主线程] 红绿灯状态:" << tl.getStateName()
            << " timer:" << tl.timer << "/" << tl.switchInterval;
    }
}

void MainWindow::addLog(const QString &msg)
{
    logTextEdit_->append(msg);
    logTextEdit_->verticalScrollBar()->setValue(
        logTextEdit_->verticalScrollBar()->maximum()
        );
}