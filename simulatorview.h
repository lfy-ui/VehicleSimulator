#ifndef SIMULATORVIEW_H
#define SIMULATORVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QList>
#include <QPointF>
#include <QPainter>
#include "shareddatapool.h"

class SimulatorView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit SimulatorView(QWidget *parent = nullptr);

    void updateScene(const VehicleState &state,
                     const QList<Obstacle> &obstacles,
                     const QList<QPointF> &trajectory,
                     const Trafficlight &trafficlight
                     );

    void resetView();
    void drawTrafficLight(const Trafficlight &tl);
signals:

    void obstacleDetected(const QString &info);
    void trajectoryGenerated(const QString &info);

private:
    QGraphicsScene *scene_;

    // 车辆
    QGraphicsRectItem *vehicle_;

    // 障碍物
    QList<QGraphicsEllipseItem*> obstacles_;

    // 轨迹
    QList<QGraphicsLineItem*> trajectoryItems_;

    // 状态文字
    QGraphicsTextItem *statusText_;

    // 检测区域
    QList<QGraphicsPathItem*> zoneItems_;

    // 道路元素
    QGraphicsRectItem *road_;
    QList<QGraphicsLineItem*> laneLines_;
    QList<QGraphicsLineItem*> roadEdges_;
    QList<QGraphicsItem*> trafficLightItems_;
    QList<QGraphicsItem*> zebraItems_;

    // 绘制函数
    void drawRoad();
    void drawVehicle(const VehicleState &state);
    void drawObstacles(const QList<Obstacle> &obstacles);
    void drawTrajectory(const QList<QPointF> &trajectory);
    void drawDetectionZone(const VehicleState &state);
    void drawTrafficlight(const Trafficlight &tl);
    //void drawZebraCrossing(double x, double y);
    void clearObstacles();
    void clearTrajectory();
    void clearZone();
    void clearTrafficLight();
    //void clearZebra();
};

#endif // SIMULATORVIEW_H