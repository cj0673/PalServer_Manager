#ifndef OTHERTHREAD_H
#define OTHERTHREAD_H

#include <QThread>
#include "RCONClient.h"
#include <QMutex>
#include <QWaitCondition>

class OtherThread : public QThread {
    Q_OBJECT

public:
    explicit OtherThread(RCONClient *client, QObject *parent = nullptr);
    void stopThread();
    void triggerRestart();

signals:
    void updateStatus(QString status);
    void restartServer();
    void reconnectrcon();

protected:
    void run() override;

private:
    RCONClient *rconClient;
    QMutex mutex;
    QWaitCondition condition;
    bool active;
    bool triggerRestartFlag;

    void performRestartOperations();
};

#endif // OTHERTHREAD_H
