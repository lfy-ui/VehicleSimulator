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

    void updateScene(const QList<VehicleState> &vehicles,
                     const QList<Obstacle> &obstacles,
                     const QList<QPointF> &trajectory,
                     const Trafficlight &trafficlight,
                     const Pedestrian &pedestrian);

    void resetView();
    void clearObstacles();
    void drawTrafficLight(const Trafficlight &tl);

signals:
    void obstacleDetected(const QString &info);
    void trajectoryGenerated(const QString &info);

private:
    QGraphicsScene *scene_;
    QList<QGraphicsRectItem*> vehicleItems_;
    QList<QGraphicsEllipseItem*> obstacles_;
    QList<QGraphicsLineItem*> trajectoryItems_;
    QGraphicsTextItem *statusText_;
    QList<QGraphicsPathItem*> zoneItems_;
    QGraphicsRectItem *road_;
    QList<QGraphicsLineItem*> laneLines_;
    QList<QGraphicsLineItem*> roadEdges_;
    QList<QGraphicsItem*> trafficLightItems_;
    QList<QGraphicsItem*> zebraItems_;
    QList<QGraphicsItem*> pedestrianItems_;
    QList<QGraphicsItem*> parkingItems_;

    void drawRoad();
    void drawSingleRoad(double yCenter, QColor color, double halfWidth);
    void drawVehicles(const QList<VehicleState> &vehicles);
    void drawObstacles(const QList<Obstacle> &obstacles);
    void drawTrajectory(const QList<QPointF> &trajectory);
    void drawDetectionZone(const VehicleState &state);
    void drawTrafficlight(const Trafficlight &tl);
    void drawZebraCrossing(double x, double y);
    void clearVehicles();
    void clearTrajectory();
    void clearZone();
    void clearTrafficLight();
    void drawPedestrian(const Pedestrian &ped);
    void clearPedestrian();
    void drawParkingSlot();
};

#endif // SIMULATORVIEW_H