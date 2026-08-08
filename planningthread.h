#ifndef PLSNNINGTHERAD_H
#define PLSNNINGTHERAD_H

#include <QObject>
#include <QThread>
#include"shareddatapool.h"

class PlanningTherad : public QThread
{
    Q_OBJECT
public:
    explicit PlanningTherad(sharedDatapool &pool,QObject *parent = nullptr);

protected:
    void run() override;

private:
    sharedDatapool &pool_;

    bool isAvoiding_ = false;
    //bool hasAvoiding_ = false;
    //double currentAvoidDirection_ = 0.0;
};

#endif // PLSNNINGTHERAD_H
