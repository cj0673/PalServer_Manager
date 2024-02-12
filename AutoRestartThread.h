#ifndef AUTORESTARTTHREAD_H
#define AUTORESTARTTHREAD_H

#include <QMutex>
#include <QThread>
#include <QDateTime>
#include <QWaitCondition>
#include "RCONClient.h"

class AutoRestartThread : public QThread {
    Q_OBJECT

public:
    explicit AutoRestartThread(RCONClient *client, QObject *parent = nullptr);
    void stopThread(); // 控制線程的停止

    // 設置自動重啟
    void setAutoRestartEnabled(bool enabled);
    void setRestartTimes(const QList<int>& times);
    void setReconnectCallback(const std::function<void()>& callback);

signals:
    void updateStatus(QString status);
    void restartServer();

protected:
    void run() override;

private:
    // 互斥鎖
    QWaitCondition condition;
    QMutex mutex;

    RCONClient* rconClient; // RCON客戶端
    std::function<void()> reconnectCallback; // 重連函數
    QList<int> restartTimes; // 存儲重啟時間列表
    QDateTime nextRestart; // 下一次重啟的時間
    void sendRestartWarning(int minutes);// 發送重啟前警告
    bool active; // 活動狀態標誌
    bool autoRestartEnabled; // 自動重啟標誌
    bool getNextRestart; // 是否要獲取下次重啟時間的標誌
    bool restartTimesUpdated; // 重啟時間更新標誌

    // 重啟和計算下次重啟時間
    QDateTime calculateNextRestartTime();
    void performRestartOperations();
};

#endif // AUTORESTARTTHREAD_H
