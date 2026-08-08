#include "simulatorview.h"
#include <QPen>
#include <QBrush>
#include <QDebug>
#include <cmath>

SimulatorView::SimulatorView(QWidget *parent)
    : QGraphicsView(parent), scene_(new QGraphicsScene(this))
{
    setScene(scene_);
    setSceneRect(-150, -100, 300, 200);
    setBackgroundBrush(Qt::white);
    setRenderHint(QPainter::Antialiasing);

    drawRoad();

    // 车辆
    vehicle_ = scene_->addRect(-8, -4, 16, 8, QPen(Qt::red, 2), QBrush(Qt::red));
    vehicle_->setPos(0, 0);
    vehicle_->setZValue(10);

    // 状态文字
    statusText_ = scene_->addText("X: 0  Y: 0  Speed: 0");
    statusText_->setPos(-140, -90);
    statusText_->setDefaultTextColor(Qt::blue);

    qDebug() << "[SimulatorView] 构造完成";
}

void SimulatorView::drawRoad()
{
    // 路面
    road_ = scene_->addRect(-150, -30, 300, 60,
                            QPen(Qt::darkGray, 2),
                            QBrush(QColor(80, 80, 80)));
    road_->setZValue(0);

    // 车道线（白色虚线）
    for (int x = -140; x < 140; x += 20) {
        QGraphicsLineItem* line = scene_->addLine(x, 0, x + 10, 0, QPen(Qt::white, 2));
        line->setZValue(1);
        laneLines_.append(line);
    }

    // 道路边线（黄色）
    QGraphicsLineItem* topLine = scene_->addLine(-150, -30, 150, -30, QPen(Qt::yellow, 3));
    QGraphicsLineItem* bottomLine = scene_->addLine(-150, 30, 150, 30, QPen(Qt::yellow, 3));
    topLine->setZValue(1);
    bottomLine->setZValue(1);
    roadEdges_.append(topLine);
    roadEdges_.append(bottomLine);

    //drawZebraCrossing(100.0, -30.0);
    double x = 110.0;
    double y = 0;
    double width = 60.0;
    double length = 12.0;
    int stripeCount = 8;
    double stripeWidth = length / stripeCount;

    QGraphicsRectItem* bg = scene_->addRect(
        x - length/2, y - width/2,
        length, width,
        QPen(Qt::darkGray, 1),
        QBrush(QColor(80, 80, 80))
        );
    bg->setZValue(0);

    for (int i = 0; i < stripeCount; i++) {
        double xPos = x - length/2 + i * stripeWidth + stripeWidth/2;
        QGraphicsRectItem* stripe = scene_->addRect(
            xPos - stripeWidth/2 + 0.2, y - width/2 + 0.2,
            stripeWidth - 0.4, width - 0.4,
            QPen(Qt::NoPen),
            QBrush(Qt::white)
            );
            stripe->setZValue(1);
    }

    // 边框
    QGraphicsLineItem* leftBorder = scene_->addLine(
        x - length/2, y - width/2,
        x - length/2, y + width/2,
        QPen(Qt::black, 1.5)
        );
    leftBorder->setZValue(1);

    QGraphicsLineItem* rightBorder = scene_->addLine(
        x + length/2, y - width/2,
        x + length/2, y + width/2,
        QPen(Qt::black, 1.5)
        );
    rightBorder->setZValue(1);

    QGraphicsTextItem* label = scene_->addText("🚶 斑马线");
    label->setPos(x + length/2 + 1, y - 2);
    label->setScale(0.5);
    label->setDefaultTextColor(Qt::white);
    label->setZValue(3);
}

void SimulatorView::drawDetectionZone(const VehicleState &state)
{
    clearZone();

    // 扇形检测区域
    QPainterPath path;
    path.moveTo(state.x, state.y);
    path.arcTo(state.x - 15, state.y - 15, 30, 30, -30, 60);
    path.closeSubpath();

    QGraphicsPathItem* zone = scene_->addPath(path, QPen(Qt::gray, 1, Qt::DashLine),
                                              QBrush(QColor(200, 200, 255, 50)));
    zone->setZValue(2);
    zoneItems_.append(zone);
}

void SimulatorView::drawObstacles(const QList<Obstacle> &obstacles)
{
    clearObstacles();

    for (const Obstacle &obs : obstacles) {
        // 只显示道路范围内的障碍物
        if (obs.y < -30 || obs.y > 30) continue;
        if (obs.x < -150 || obs.x > 150) continue;

        double dx = obs.x - vehicle_->pos().x();
        double dy = obs.y - vehicle_->pos().y();
        double dist = sqrt(dx*dx + dy*dy);

        QColor color;
        if (dist < 5.0) {
            color = Qt::red;
        } else if (dist < 15.0) {
            color = QColor(255, 165, 0);
        } else {
            color = Qt::yellow;
        }

        QGraphicsEllipseItem *item = scene_->addEllipse(-5, -5, 10, 10,
                                                        QPen(Qt::black, 1),
                                                        QBrush(color));
        item->setPos(obs.x, obs.y);
        item->setZValue(5);
        obstacles_.append(item);
    }
}

void SimulatorView::drawTrajectory(const QList<QPointF> &trajectory)
{
    clearTrajectory();

    if (trajectory.size() < 2) return;

    QPen pen(Qt::blue, 2, Qt::DashLine);
    for (int i = 0; i < trajectory.size() - 1; i++) {
        QGraphicsLineItem* line = scene_->addLine(trajectory[i].x(), trajectory[i].y(),
                                                  trajectory[i+1].x(), trajectory[i+1].y(),
                                                  pen);
        line->setZValue(3);
        trajectoryItems_.append(line);
    }
}

void SimulatorView::drawTrafficLight(const Trafficlight &tl)
{
    clearTrafficLight();

    if (tl.x < -150 || tl.x > 150) return;

    // 灯柱
    QGraphicsLineItem* pole = scene_->addLine(tl.x, tl.y, tl.x, tl.y + 10, QPen(Qt::darkGray, 2));
    trafficLightItems_.append(pole);

    // 灯体
    QGraphicsRectItem* body = scene_->addRect(tl.x - 4, tl.y - 10, 8, 12,
                                              QPen(Qt::black, 1),
                                              QBrush(Qt::darkGray));
    trafficLightItems_.append(body);

    // 灯的颜色
    QColor color = tl.getColor();
    QGraphicsEllipseItem* light = scene_->addEllipse(tl.x - 2.5, tl.y - 7, 5, 5,
                                                     QPen(Qt::black, 1),
                                                     QBrush(color));
    trafficLightItems_.append(light);

    // 状态文字
    QString stateText;
    switch (tl.state) {
    case TrafficlightState::Red:    stateText = "红灯 停"; break;
    case TrafficlightState::Yellow: stateText = "黄灯 慢"; break;
    case TrafficlightState::Green:  stateText = "绿灯 行"; break;
    }

    QGraphicsTextItem* label = scene_->addText(stateText);
    label->setPos(tl.x - 10, tl.y - 22);
    label->setScale(0.7);
    label->setDefaultTextColor(color);
    trafficLightItems_.append(label);
}

void SimulatorView::clearObstacles()
{
    for (QGraphicsEllipseItem *item : obstacles_) {
        if (item) {
            scene_->removeItem(item);
            delete item;
        }
    }
    obstacles_.clear();
}

void SimulatorView::clearTrajectory()
{
    for (QGraphicsLineItem *item : trajectoryItems_) {
        if (item) {
            scene_->removeItem(item);
            delete item;
        }
    }
    trajectoryItems_.clear();
}

void SimulatorView::clearZone()
{
    for (QGraphicsPathItem *item : zoneItems_) {
        if (item) {
            scene_->removeItem(item);
            delete item;
        }
    }
    zoneItems_.clear();
}

void SimulatorView::clearTrafficLight()
{
    for (QGraphicsItem *item : trafficLightItems_) {
        if (item) {
            scene_->removeItem(item);
            delete item;
        }
    }
    trafficLightItems_.clear();
}

// void SimulatorView::clearZebra()
// {
//     for (QGraphicsItem* item : zebraItems_) {
//         if (item) {
//             scene_->removeItem(item);
//             delete item;
//         }
//     }
//     zebraItems_.clear();
// }

void SimulatorView::resetView()
{
    // 删除车辆
    if (vehicle_) {
        scene_->removeItem(vehicle_);
        delete vehicle_;
        vehicle_ = nullptr;
    }

    vehicle_ = scene_->addRect(-8, -4, 16, 8, QPen(Qt::red, 2), QBrush(Qt::red));
    vehicle_->setPos(0, 0);
    vehicle_->setZValue(10);

    // 删除状态文字
    if (statusText_) {
        scene_->removeItem(statusText_);
        delete statusText_;
        statusText_ = nullptr;
    }

    statusText_ = scene_->addText("X: 0  Y: 0  Speed: 0");
    statusText_->setPos(-140, -90);
    statusText_->setDefaultTextColor(Qt::blue);

    // 清空所有元素
    clearObstacles();
    clearTrajectory();
    clearZone();
    clearTrafficLight();
    //clearZebra();

    // 重新绘制道路
    drawRoad();

    qDebug() << "[SimulatorView] resetView 完成";
}

void SimulatorView::updateScene(const VehicleState &state,
                                const QList<Obstacle> &obstacles,
                                const QList<QPointF> &trajectory,
                                const Trafficlight &trafficlight
                                )
{
    if (!vehicle_) return;

    // 更新车辆位置
    vehicle_->setPos(state.x, state.y);

    // 绘制检测区域
    drawDetectionZone(state);

    // 绘制障碍物
    drawObstacles(obstacles);

    // 绘制轨迹
    drawTrajectory(trajectory);

    // 绘制红绿灯
    drawTrafficLight(trafficlight);

    // 更新状态文字
    statusText_->setPlainText(QString("X: %1  Y: %2  Speed: %3")
                                  .arg(state.x, 0, 'f', 1)
                                  .arg(state.y, 0, 'f', 1)
                                  .arg(state.speed, 0, 'f', 1));
}