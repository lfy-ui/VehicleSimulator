#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QScrollBar>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)

{
     //qDebug() << "[MainWindow] 1. 开始构造";
    //ui->setupUi(this);
     // qDebug() << "[MainWindow] 2. setupUi 完成";


    QWidget* central = new QWidget(this);
    setCentralWidget(central);
     //qDebug() << "[MainWindow] 3. centralWidget 创建完成";


    QHBoxLayout* mainLayout = new QHBoxLayout(central);
     //qDebug() << "[MainWindow] 4. mainLayout 创建完成";

     QVBoxLayout* leftLayout = new QVBoxLayout();
         //qDebug() << "[MainWindow] 5. leftLayout 创建完成";

    simulatorView_ = new SimulatorView(this);
    leftLayout->addWidget(simulatorView_);
    //qDebug() << "[MainWindow] 6. SimulatorView 创建完成";


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
    //qDebug() << "[MainWindow] 7. 控制栏创建完成";
    mainLayout->addLayout(leftLayout, 3);
    //qDebug() << "[MainWindow] 8. 左侧布局添加完成";

    QVBoxLayout* rightLayout = new QVBoxLayout();
     //qDebug() << "[MainWindow] 9. rightLayout 创建完成";
    QLabel* statusLabel = new QLabel("就绪", this);
    QLabel* speedLabel = new QLabel("速度: 0.0 m/s", this);
    QLabel* yawLabel = new QLabel("航向: 0.00 rad", this);
    QLabel* obstacleLabel = new QLabel("障碍物: 0", this);
    QLabel* decisionLabel = new QLabel("决策: 正常行驶", this);

    QTextEdit* logTextEdit = new QTextEdit(this);
    logTextEdit->setReadOnly(true);
    logTextEdit->setMaximumHeight(200);

    rightLayout->addWidget(new QLabel("🚗 车辆状态", this));
    rightLayout->addWidget(statusLabel);
    rightLayout->addWidget(speedLabel);
    rightLayout->addWidget(yawLabel);
    rightLayout->addWidget(obstacleLabel);
    rightLayout->addWidget(decisionLabel);
    rightLayout->addWidget(new QLabel("📋 日志", this));
    rightLayout->addWidget(logTextEdit);
    //qDebug() << "[MainWindow] 10. 右侧面板创建完成";

    mainLayout->addLayout(rightLayout, 1);
    //qDebug() << "[MainWindow] 11. 右侧布局添加完成";

    startBtn_ = startBtn;
    pauseBtn_ = pauseBtn;
    resetBtn_ = resetBtn;
    statusLabel_ = statusLabel;
    speedLabel_ = speedLabel;
    yawLabel_ = yawLabel;
    obstacleLabel_ = obstacleLabel;
    decisionLabel_ = decisionLabel;
    logTextEdit_ = logTextEdit;
    stepLabel_ = stepLabel;
    //qDebug() << "[MainWindow] 12. 控件指针保存完成";

    connect(startBtn, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(pauseBtn, &QPushButton::clicked, this, &MainWindow::onPauseClicked);
    connect(resetBtn, &QPushButton::clicked, this, &MainWindow::onResetClicked);
    //qDebug() << "[MainWindow] 13. 信号连接完成";

    pauseBtn->setEnabled(false);
    resetBtn->setEnabled(false);
     //qDebug() << "[MainWindow] 14. 按钮状态设置完成";

    pool_ = new sharedDatapool();
    //qDebug() << "[MainWindow] 15. pool 创建完成";


    updateTimer_ = new QTimer(this);
    connect(updateTimer_, &QTimer::timeout, this, &MainWindow::onUpdateTimer);
    //qDebug() << "[MainWindow] 16. 定时器创建完成";

    addLog("[INFO] 仿真平台已启动");
   // qDebug() << "[MainWindow] 17. 构造函数完成";
}

MainWindow::~MainWindow()
{
    if (updateTimer_) {
        updateTimer_->stop();
    }
    if (pool_) {
        pool_->stopSystem();
    }
    //if (isRunning_) {
        //pool_->stopSystem();
        if (locThread_) { locThread_->wait(); delete locThread_; }
        if (perThread_) { perThread_->wait(); delete perThread_; }
        if (planThread_) { planThread_->wait(); delete planThread_; }
        if (monThread_) { monThread_->wait(); delete monThread_; }
   // }
        if (pool_) {
        delete pool_;
        pool_ = nullptr;
}
    //delete ui;
}

void MainWindow::onStartClicked()
{
    if (isRunning_) return;

    isRunning_ = true;
    startBtn_->setEnabled(false);
    pauseBtn_->setEnabled(true);
    resetBtn_->setEnabled(false);
    statusLabel_->setText("运行中");

    delete pool_;
    pool_ = new sharedDatapool();

    locThread_ = new LocalizationThread(*pool_, this);
    perThread_ = new PerceptionThread(*pool_, this);
    planThread_ = new PlanningTherad(*pool_, this);
    monThread_ = new MonitorThread(*pool_, this);

    locThread_->start();
    perThread_->start();
    planThread_->start();
    monThread_->start();

    updateTimer_->start(50);

    addLog("[INFO] 系统启动");
}

void MainWindow::onPauseClicked()
{
    if (!isRunning_) return;

    isRunning_ = false;
    startBtn_->setEnabled(true);
    pauseBtn_->setEnabled(false);
    resetBtn_->setEnabled(true);
    statusLabel_->setText("已暂停");
    if (updateTimer_) {
        updateTimer_->stop();
    }
    if (pool_) {
        pool_->stopSystem();
    }

    if (locThread_) { locThread_->wait(); delete locThread_; locThread_ = nullptr; }
    if (perThread_) { perThread_->wait(); delete perThread_; perThread_ = nullptr; }
    if (planThread_) { planThread_->wait(); delete planThread_; planThread_ = nullptr; }
    if (monThread_) { monThread_->wait(); delete monThread_; monThread_ = nullptr; }

    addLog("[INFO] 系统暂停");
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
    pool_->updateTrafficLight();
    static int printCounter = 0;
    printCounter++;
    if (printCounter % 10 == 0) {
        Trafficlight tl = pool_->getTrafficLight();
        qDebug() << "[主线程] 红绿灯状态:" << tl.getStateName()
                 << " timer:" << tl.timer << "/" << tl.switchInterval;
    }

    VehicleState state = pool_->get();
    QList<Obstacle> obstacles = pool_->getObstacles();
    Trafficlight tl = pool_->getTrafficLight();

    QList<QPointF> trajectory;
    for (int i = 0; i < 20; i++) {
        trajectory.append(QPointF(state.x + i * 0.5, state.y + sin(i * 0.2) * 0.5));
    }

    simulatorView_->updateScene(state, obstacles, trajectory,tl);

    speedLabel_->setText("速度: " + QString::number(state.speed, 'f', 1) + " m/s");
    yawLabel_->setText("航向: " + QString::number(state.yaw, 'f', 2) + " rad");
    obstacleLabel_->setText("障碍物: " + QString::number(obstacles.size()));

    ControlCommand cmd = pool_->getControlCommand();
    decisionLabel_->setText("决策: " + cmd.reason);

    stepLabel_->setText("步长: 50ms");

    for (const auto& obs : obstacles) {
        double dx = obs.x - state.x;
        double dy = obs.y - state.y;
        if (sqrt(dx*dx + dy*dy) < 5.0) {
            addLog("[WARN] 检测到近距离障碍物!");
            break;
        }
    }
}

void MainWindow::addLog(const QString &msg)
{
    logTextEdit_->append(msg);
    logTextEdit_->verticalScrollBar()->setValue(
        logTextEdit_->verticalScrollBar()->maximum()
        );
}