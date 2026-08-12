#include "simulatorview.h"
#include <QPen>
#include <QBrush>
#include <QDebug>
#include <cmath>

SimulatorView::SimulatorView(QWidget *parent)
    : QGraphicsView(parent), scene_(new QGraphicsScene(this))
{
    setScene(scene_);
    setSceneRect(-150, -100, 450, 200);
    setBackgroundBrush(Qt::white);
    setRenderHint(QPainter::Antialiasing);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    drawRoad();

    statusText_ = scene_->addText("X: 0  Y: 0  Speed: 0");
    statusText_->setPos(-140, -90);
    statusText_->setDefaultTextColor(Qt::blue);

    qDebug() << "[SimulatorView] 构造完成（双车道，扇形检测区域，停车位）";
}

void SimulatorView::drawRoad()
{
    // 车道1：中心 -20，半宽 20
    drawSingleRoad(-20, QColor(60, 60, 70), 20.0);
    // 车道2：中心 20，半宽 20
    drawSingleRoad(20, QColor(60, 60, 70), 20.0);

    drawZebraCrossing(110.0, 0);
    drawParkingSlot();  // 绘制停车位
}

void SimulatorView::drawSingleRoad(double yCenter, QColor color, double halfWidth)
{
    double width = halfWidth * 2;
    QGraphicsRectItem* road = scene_->addRect(-150, yCenter - halfWidth, 450, width,
                                              QPen(Qt::NoPen),
                                              QBrush(color));
    road->setZValue(0);

    for (int x = -140; x < 290; x += 20) {
        QGraphicsLineItem* line = scene_->addLine(x, yCenter, x + 10, yCenter, QPen(Qt::white, 1.5));
        line->setZValue(1);
        laneLines_.append(line);
    }

    QGraphicsLineItem* topLine = scene_->addLine(-150, yCenter - halfWidth, 300, yCenter - halfWidth, QPen(Qt::yellow, 2));
    QGraphicsLineItem* bottomLine = scene_->addLine(-150, yCenter + halfWidth, 300, yCenter + halfWidth, QPen(Qt::yellow, 2));
    topLine->setZValue(1);
    bottomLine->setZValue(1);
    roadEdges_.append(topLine);
    roadEdges_.append(bottomLine);
}

void SimulatorView::drawZebraCrossing(double x, double y)
{
    double width = 80.0;
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
    label->setPos(x + length/2 + 2, y - 2);
    label->setScale(0.5);
    label->setDefaultTextColor(Qt::white);
    label->setZValue(3);
}

void SimulatorView::drawVehicles(const QList<VehicleState> &vehicles)
{
    clearVehicles();
    for (const VehicleState& state : vehicles) {
        QColor color = (state.id == 1) ? Qt::red : Qt::blue;
        QGraphicsRectItem* car = scene_->addRect(-8, -4, 16, 8,
                                                 QPen(color, 2),
                                                 QBrush(color));
        car->setPos(state.x, state.y);
        car->setZValue(10);
        vehicleItems_.append(car);
    }
}

void SimulatorView::clearVehicles()
{
    for (QGraphicsRectItem* item : vehicleItems_) {
        if (item) {
            scene_->removeItem(item);
            delete item;
        }
    }
    vehicleItems_.clear();
}

void SimulatorView::drawObstacles(const QList<Obstacle> &obstacles)
{
    clearObstacles();
    QPointF carPos = (vehicleItems_.isEmpty()) ? QPointF(0, 0) : vehicleItems_.first()->pos();
    for (const Obstacle &obs : obstacles) {
        if (obs.y < -45 || obs.y > 45) continue;
        if (obs.x < -150 || obs.x > 300) continue;
        double dx = obs.x - carPos.x();
        double dy = obs.y - carPos.y();
        double dist = sqrt(dx*dx + dy*dy);
        QColor color;
        if (dist < 5.0) color = Qt::red;
        else if (dist < 15.0) color = QColor(255, 165, 0);
        else color = Qt::yellow;
        QGraphicsEllipseItem *item = scene_->addEllipse(-5, -5, 10, 10,
                                                        QPen(Qt::black, 1),
                                                        QBrush(color));
        item->setPos(obs.x, obs.y);
        item->setZValue(5);
        obstacles_.append(item);
    }
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

void SimulatorView::drawDetectionZone(const VehicleState &state)
{
    QPainterPath path;
    path.moveTo(state.x, state.y);
    path.arcTo(state.x - 15, state.y - 15, 30, 30, -30, 60);
    path.closeSubpath();

    QColor fillColor = (state.id == 1) ? QColor(255, 0, 0, 50) : QColor(0, 0, 255, 50);
    QGraphicsPathItem* zone = scene_->addPath(path, QPen(Qt::gray, 1, Qt::DashLine),
                                              QBrush(fillColor));
    zone->setZValue(2);
    zoneItems_.append(zone);
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

void SimulatorView::drawTrafficLight(const Trafficlight &tl)
{
    clearTrafficLight();
    if (tl.x < -150 || tl.x > 300) return;
    QGraphicsLineItem* pole = scene_->addLine(tl.x, tl.y, tl.x, tl.y + 10, QPen(Qt::darkGray, 2));
    trafficLightItems_.append(pole);
    QGraphicsRectItem* body = scene_->addRect(tl.x - 4, tl.y - 10, 8, 12,
                                              QPen(Qt::black, 1),
                                              QBrush(Qt::darkGray));
    trafficLightItems_.append(body);
    QColor color = tl.getColor();
    QGraphicsEllipseItem* light = scene_->addEllipse(tl.x - 2.5, tl.y - 7, 5, 5,
                                                     QPen(Qt::black, 1),
                                                     QBrush(color));
    trafficLightItems_.append(light);
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

void SimulatorView::drawPedestrian(const Pedestrian &ped)
{
    clearPedestrian();
    QBrush brush(Qt::green);
    QPen pen(Qt::black, 1);

    QGraphicsEllipseItem* head = scene_->addEllipse(-3, -8, 6, 6, pen, brush);
    head->setPos(ped.x, ped.y);
    head->setZValue(20);
    pedestrianItems_.append(head);

    QGraphicsRectItem* body = scene_->addRect(-2, -2, 4, 6, pen, brush);
    body->setPos(ped.x, ped.y);
    body->setZValue(20);
    pedestrianItems_.append(body);

    QGraphicsTextItem* label = scene_->addText("🚶");
    label->setPos(ped.x - 8, ped.y - 16);
    label->setScale(0.8);
    label->setZValue(21);
    pedestrianItems_.append(label);
}

void SimulatorView::clearPedestrian()
{
    for (QGraphicsItem *item : pedestrianItems_) {
        if (item) {
            scene_->removeItem(item);
            delete item;
        }
    }
    pedestrianItems_.clear();
}

//停车位绘制
void SimulatorView::drawParkingSlot()
{
    // 停车位在车道1外侧（y=-40），x=170
    double cx = 170.0;
    double cy = -40.0;
    double halfW = 10.0;
    double halfH = 4.0;

    QGraphicsRectItem* bg = scene_->addRect(
        cx - halfW, cy - halfH,
        halfW * 2, halfH * 2,
        QPen(Qt::blue, 1.5, Qt::DashLine),
        QBrush(QColor(200, 200, 255, 80))
        );
    bg->setZValue(4);
    parkingItems_.append(bg);

    QGraphicsTextItem* label = scene_->addText("🅿️ 停车位");
    label->setPos(cx - 15, cy - 8);
    label->setScale(0.5);
    label->setDefaultTextColor(Qt::blue);
    label->setZValue(5);
    parkingItems_.append(label);
}

void SimulatorView::resetView()
{
    clearVehicles();
    clearObstacles();
    clearTrajectory();
    clearZone();
    clearTrafficLight();
    clearPedestrian();

    // 清理停车位
    for (QGraphicsItem *item : parkingItems_) {
        if (item) {
            scene_->removeItem(item);
            delete item;
        }
    }
    parkingItems_.clear();

    drawRoad();
    if (statusText_) {
        statusText_->setPlainText("X: 0  Y: 0  Speed: 0");
    }
    centerOn(0, 0);
    qDebug() << "[SimulatorView] resetView 完成";
}

void SimulatorView::updateScene(const QList<VehicleState> &vehicles,
                                const QList<Obstacle> &obstacles,
                                const QList<QPointF> &trajectory,
                                const Trafficlight &trafficlight,
                                const Pedestrian &pedestrian)
{
    // 清理旧的扇形区域
    clearZone();

    drawVehicles(vehicles);

    // 为所有车辆绘制检测区域
    for (const VehicleState& state : vehicles) {
        drawDetectionZone(state);
    }

    drawObstacles(obstacles);
    drawTrajectory(trajectory);
    drawTrafficLight(trafficlight);
    drawPedestrian(pedestrian);

    if (!vehicles.isEmpty()) {
        const VehicleState& state = vehicles.first();
        statusText_->setPlainText(QString("车辆1 X: %1  Y: %2  Speed: %3")
                                      .arg(state.x, 0, 'f', 1)
                                      .arg(state.y, 0, 'f', 1)
                                      .arg(state.speed, 0, 'f', 1));
    }
}