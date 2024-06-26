#include "AutoRestartThread.h"
#include <QDateTime>

AutoRestartThread::AutoRestartThread(RCONClient *client, QObject *parent)
    : QThread(parent), rconClient(client), active(true), autoRestartEnabled(false), getNextRestart(true), restartTimesUpdated(false) {}

void AutoRestartThread::setAutoRestartEnabled(bool enabled) {
    QMutexLocker locker(&mutex);
    autoRestartEnabled = enabled;
    if (autoRestartEnabled) {
        condition.wakeOne();
    }
}

void AutoRestartThread::setRestartTimes(const QList<int>& times) {
    QMutexLocker locker(&mutex);
    restartTimes.clear();
    for (int time : times) {
        restartTimes.append(time);
    }

    nextRestart = QDateTime();
    getNextRestart = true;
    restartTimesUpdated = true;
}

void AutoRestartThread::setReconnectCallback(const std::function<void()>& callback) {
    reconnectCallback = callback;
}

void AutoRestartThread::run() {
    const QVector<int> warningMinutes = {30, 20, 10, 7, 5, 4, 3, 2, 1};

    while (active) {
        {
            QMutexLocker locker(&mutex);
            while (active && !autoRestartEnabled) {
                condition.wait(&mutex);
            }
        }

        if (getNextRestart || !nextRestart.isValid()) {
            nextRestart = calculateNextRestartTime();
            getNextRestart = false;
        }

        QDateTime now = QDateTime::currentDateTime();
        qint64 secondsUntilRestart = now.secsTo(nextRestart);

        if (secondsUntilRestart <= 0 && nextRestart.isValid()) {
            performRestartOperations();
            getNextRestart = true;
            continue;
        }

        int hoursUntilRestart = secondsUntilRestart / 3600;
        int minutesUntilRestart = ((secondsUntilRestart % 3600) / 60) + 1;

        emit updateStatus(QString("距離下一次重啟還有 %1 小時 %2 分鐘").arg(hoursUntilRestart).arg(minutesUntilRestart));

        if (warningMinutes.contains(minutesUntilRestart) && (now.time().hour() + 1) % 24 == nextRestart.time().hour()) {
            sendRestartWarning(minutesUntilRestart);
        }

        for (int i = 0; i < 600; ++i) {
            if (!active || !autoRestartEnabled || restartTimesUpdated) break;
            QThread::msleep(100);
        }

        if (restartTimesUpdated) {
            restartTimesUpdated = false;
            continue;
        }
    }
}

void AutoRestartThread::performRestartOperations() {
    reconnectCallback();
    std::string response;
    if (!rconClient->SendCommand("save", response)) {
        emit updateStatus("無法發送指令或接收回應");
        return;
    }
    if (!rconClient->SendCommand("shutdown 30 Server_restart_in_30_sec", response)) {
        emit updateStatus("無法發送指令或接收回應");
        return;
    } else {
        emit updateStatus("已下達關閉伺服器命令，伺服器將於90秒後重新啟動");
    }

    QThread::sleep(90);

    emit restartServer();
    emit updateStatus("伺服器已完成重啟");
    QThread::sleep(10);
    reconnectCallback();
}

void AutoRestartThread::sendRestartWarning(int minutes) {
    reconnectCallback();
    std::string warning = "broadcast Server_restart_in_" + std::to_string(minutes) + "_min";
    std::string response;
    if (!rconClient->SendCommand(warning, response)) {
        emit updateStatus("無法發送重啟提示");
    }
}

QDateTime AutoRestartThread::calculateNextRestartTime() {
    QDateTime now = QDateTime::currentDateTime();

    if (restartTimes.isEmpty()) {
        return QDateTime();
    }

    foreach (int hour, restartTimes) {
        QDateTime restartTimeToday = QDateTime(now.date(), QTime(hour, 0));
        if (now < restartTimeToday) {
            return restartTimeToday;
        }
    }

    return QDateTime(now.date().addDays(1), QTime(restartTimes.first(), 0));
}

void AutoRestartThread::stopThread() {
    {
        QMutexLocker locker(&mutex);
        active = false;
        condition.wakeOne();
    }
    if (!isRunning()) {
        return;
    }
    wait();
}
