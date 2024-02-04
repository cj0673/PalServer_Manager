#ifndef AUTORESTARTTHREAD_H
#define AUTORESTARTTHREAD_H

#include <QThread>
#include <QDateTime>
#include "RCONClient.h"
#include <QWaitCondition>
#include <QMutex>

class AutoRestartThread : public QThread {
    Q_OBJECT

public:
    AutoRestartThread(RCONClient *client, QObject *parent = nullptr);
    void stopThread();
    void setAutoRestartEnabled(bool enabled);
    void setReconnectCallback(const std::function<void()>& callback);
    void setRestartTimes(const QSet<int>& times);

signals:
    void updateStatus(QString status);
    void restartServer();

protected:
    void run() override;

private:
    RCONClient *rconClient;
    bool active;
    bool autoRestartEnabled;
    bool getNextRestart;
    QDateTime nextRestart;

    QWaitCondition condition;
    QMutex mutex;

    void performRestartOperations();
    QDateTime calculateNextRestartTime();

    void sendRestartWarning(int minutes);

    std::function<void()> reconnectCallback;

    QList<int> restartTimes;
};

#endif // AUTORESTARTTHREAD_H
