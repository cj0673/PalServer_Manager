#include "OtherThread.h"
#include <QDateTime>

OtherThread::OtherThread(RCONClient *client, QObject *parent)
    : QThread(parent), rconClient(client), active(true), triggerRestartFlag(false) {}

void OtherThread::triggerRestart() {
    QMutexLocker locker(&mutex);
    triggerRestartFlag = true;
    condition.wakeOne();
}

void OtherThread::run() {
    while (active) {
        {
            QMutexLocker locker(&mutex);
            while (active && !triggerRestartFlag) {
                condition.wait(&mutex);
            }
        }

        if (active && triggerRestartFlag) {
            performRestartOperations();
            triggerRestartFlag = false;
        }
    }
}

void OtherThread::performRestartOperations() {
    std::string response;
    if (!rconClient->SendCommand("save", response)) {
        emit updateStatus("無法發送指令或接收回應");
        return;
    }
    if (!rconClient->SendCommand("shutdown 5 Server_restart", response)) {
        emit updateStatus("無法發送指令或接收回應");
        return;
    }
    emit updateStatus("已發送關閉伺服器命令，將於90秒後重新啟動");

    QThread::sleep(90);

    emit restartServer();
    emit updateStatus("伺服器已完成重啟");
    QThread::sleep(10);
    emit reconnectrcon();
}

void OtherThread::stopThread() {
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
