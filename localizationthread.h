#ifndef LOCALIZATIONTHREAD_H
#define LOCALIZATIONTHREAD_H

#include <QObject>
#include <QThread>
#include"shareddatapool.h"

class LocalizationThread : public QThread
{
    Q_OBJECT
public:
    explicit LocalizationThread( sharedDatapool &pool ,QObject *parent = nullptr);

protected:
    void run() override;

private:
    sharedDatapool &pool_;

};

#endif // LOCALIZATIONTHREAD_H
