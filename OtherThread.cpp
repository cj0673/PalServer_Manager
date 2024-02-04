#include "OtherThread.h"
#include <QDateTime>

OtherThread::OtherThread(RCONClient *client, QObject *parent)
    : QThread(parent), rconClient(client), active(true), triggerRestartFlag(false) {}

void OtherThread::triggerRestart() {
    triggerRestartFlag = true;
}

void OtherThread::run() {
    while (active) {
        if (triggerRestartFlag) {
            std::string response;
            if (!rconClient->SendCommand("save", response)) {
                emit updateStatus("無法發送指令或接收回應");
                triggerRestartFlag = false;
                continue;
            }
            if (!rconClient->SendCommand("shutdown 5 Server_restart", response)) {
                emit updateStatus("無法發送指令或接收回應");
                triggerRestartFlag = false;
                continue;
            } else {
                emit updateStatus("已發送關閉伺服器命令，將於90秒後重新啟動");
            }

            QThread::sleep(90);

            emit restartServer();
            emit updateStatus("伺服器已完成重啟");
            QThread::sleep(10);
            emit reconnectrcon();

            triggerRestartFlag = false;
        }

        QThread::msleep(100);
    }
}

void OtherThread::stopThread() {
    active = false;
    if (!isRunning()) {
        return;
    }
    wait();
}
