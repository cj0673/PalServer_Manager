#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QMessageBox>
#include <QInputDialog>

// JSON處理
#include <QJsonDocument>
#include <QJsonObject>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    loadSettingsFromJson();
    initializeAutoRestartThread();
    initializeOtherThread();
}

MainWindow::~MainWindow()
{
    saveSettingsToJson();
    autoRestartThread->stopThread();
    otherThread->stopThread();
    delete ui;
}

void MainWindow::saveSettingsToJson()
{
    QJsonObject rconSettingsObject;
    rconSettingsObject["rcon_ip"] = ui->rcon_ip_lineEdit->text();
    rconSettingsObject["rcon_port"] = ui->rcon_port_lineEdit->text().toInt();
    rconSettingsObject["rcon_password"] = ui->rcon_password_lineEdit->text();

    QJsonObject rootObject;
    rootObject["rcon_settings"] = rconSettingsObject;

    QJsonDocument settingsDoc(rootObject);
    QFile file("PalServer_Manager.json");
    if (file.open(QFile::WriteOnly)) {
        file.write(settingsDoc.toJson());
        file.close();
    }
}

void MainWindow::loadSettingsFromJson() {
    QFile file("PalServer_Manager.json");
    if (file.open(QFile::ReadOnly)) {
        QByteArray rawData = file.readAll();
        file.close();

        QJsonDocument settingsDoc(QJsonDocument::fromJson(rawData));
        QJsonObject rootObject = settingsDoc.object();
        QJsonObject rconSettingsObject = rootObject["rcon_settings"].toObject();

        ui->rcon_ip_lineEdit->setText(rconSettingsObject["rcon_ip"].toString());
        ui->rcon_port_lineEdit->setText(QString::number(rconSettingsObject["rcon_port"].toInt()));
        ui->rcon_password_lineEdit->setText(rconSettingsObject["rcon_password"].toString());
    }
}

void MainWindow::initializeAutoRestartThread() {
    autoRestartThread = new AutoRestartThread(&rconClient, this);
    autoRestartThread->start();
    connect(autoRestartThread, &AutoRestartThread::updateStatus, this, [&](QString status) {
        ui->function_status->setText(status);
    });
    connect(autoRestartThread, &AutoRestartThread::restartServer, this, [&]() {
        QString executablePath = QDir::currentPath() + "/steamapps/common/PalServer/PalServer.exe";
        QProcess::startDetached(executablePath);
    });
    autoRestartThread->setReconnectCallback([this]() {
        QString ip = ui->rcon_ip_lineEdit->text();
        int port = ui->rcon_port_lineEdit->text().toInt();
        QString password = ui->rcon_password_lineEdit->text();

        std::string ip_str = ip.toStdString();
        std::string password_str = password.toStdString();

        if (!rconClient.Connect(ip_str, port, password_str)) {
            ui->connect_rcon_pushButton->setDisabled(false);
            ui->rcon_response_textBrowser->setText("無法連接到 RCON 伺服器");
        }
    });
}

void MainWindow::initializeOtherThread() {
    otherThread = new OtherThread(&rconClient, this);
    otherThread->start();
    connect(otherThread, &OtherThread::updateStatus, this, [&](QString status) {
        ui->rcon_response_textBrowser->setText(status);
    });
    connect(otherThread, &OtherThread::restartServer, this, [&]() {
        QString executablePath = QDir::currentPath() + "/steamapps/common/PalServer/PalServer.exe";
        QProcess::startDetached(executablePath);
    });
    connect(otherThread, &OtherThread::reconnectrcon, this, [&]() {
        QString ip = ui->rcon_ip_lineEdit->text();
        int port = ui->rcon_port_lineEdit->text().toInt();
        QString password = ui->rcon_password_lineEdit->text();

        std::string ip_str = ip.toStdString();
        std::string password_str = password.toStdString();

        if (!rconClient.Connect(ip_str, port, password_str)) {
            ui->connect_rcon_pushButton->setDisabled(false);
            ui->rcon_response_textBrowser->setText("無法連接到 RCON 伺服器");
        }
    });
}

void MainWindow::on_connect_rcon_pushButton_clicked()
{
    QString ip = ui->rcon_ip_lineEdit->text();
    int port = ui->rcon_port_lineEdit->text().toInt();
    QString password = ui->rcon_password_lineEdit->text();

    std::string ip_str = ip.toStdString();
    std::string password_str = password.toStdString();

    if (rconClient.Connect(ip_str, port, password_str)) {
        ui->connect_rcon_pushButton->setDisabled(true);
        ui->rcon_ip_lineEdit->setReadOnly(true);
        ui->rcon_port_lineEdit->setReadOnly(true);
        ui->rcon_password_lineEdit->setReadOnly(true);
        ui->rcon_response_textBrowser->setText("連接並認證成功，輸入 RCON 指令");
    } else {
        ui->rcon_response_textBrowser->setText("無法連接到 RCON 伺服器");
    }
}

void MainWindow::on_connect_rcon_command_info_clicked()
{
    std::string response;
    if (rconClient.SendCommand("info", response)) {
        ui->rcon_response_textBrowser->setText(QString(response.c_str()));
    }
    else {
        ui->connect_rcon_pushButton->setDisabled(false);
        ui->rcon_response_textBrowser->setPlainText("無法發送指令或接收回應");
    }
}

void MainWindow::on_connect_rcon_command_showplayers_clicked()
{
    std::string response;
    if (rconClient.SendCommand("showplayers", response)) {
        int lineCount = 0;
        size_t pos = 0;
        while ((pos = response.find('\n', pos)) != std::string::npos) {
            if (pos + 1 < response.size() && response[pos + 1] != '\n') {
                lineCount++;
            }
            pos++;
        }

        size_t newlinePos = response.find('\n');
        if (lineCount != 0) {
            response.replace(0, newlinePos, "當前在線人數：" + std::to_string(lineCount) + "\n\n資料排序為：名稱, playeruid, steamid");
        } else {
            response.replace(0, newlinePos, "當前在線人數：" + std::to_string(lineCount));
        }
        ui->rcon_response_textBrowser->setText(QString::fromStdString(response));
    }
    else {
        ui->connect_rcon_pushButton->setDisabled(false);
        ui->rcon_response_textBrowser->setText("無法發送指令或接收回應");
    }
}

void MainWindow::on_connect_rcon_command_save_clicked()
{
    std::string response;
    if (rconClient.SendCommand("save", response)) {
        ui->rcon_response_textBrowser->setText(QString(response.c_str()));
    }
    else {
        ui->connect_rcon_pushButton->setDisabled(false);
        ui->rcon_response_textBrowser->setText("無法發送指令或接收回應");
    }
}

void MainWindow::on_connect_rcon_command_shutdown_clicked()
{
    QMessageBox::StandardButton result = QMessageBox::question(this, "確認", "您確定要關閉伺服器嗎？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (result == QMessageBox::Yes)
    {
        std::string response;
        if (rconClient.SendCommand("shutdown 3 Server_shutdown", response)) {
            ui->connect_rcon_pushButton->setDisabled(false);
            ui->rcon_response_textBrowser->setText(QString(response.c_str()));
        }
        else {
            ui->connect_rcon_pushButton->setDisabled(false);
            ui->rcon_response_textBrowser->setText("無法發送指令或接收回應");
        }
    }
}

void MainWindow::on_connect_rcon_command_broadcast_clicked()
{
    bool ok;
    QString userInput = QInputDialog::getText(this, "輸入文字", "請輸入要廣播的內容:", QLineEdit::Normal, "", &ok);

    if (ok && !userInput.isEmpty()) {
        std::string response;
        std::string utf8Input = userInput.toUtf8().constData();
        std::string rconCommand = "broadcast " + utf8Input;

        if (rconClient.SendCommand(rconCommand, response)) {
            ui->rcon_response_textBrowser->setText(QString(response.c_str()));
        }
        else {
            ui->connect_rcon_pushButton->setDisabled(false);
            ui->rcon_response_textBrowser->setText("無法發送指令或接收回應");
        }
    }
}

void MainWindow::on_server_restart_clicked()
{
    QMessageBox::StandardButton result = QMessageBox::question(this, "確認", "您確定要重啟伺服器嗎？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (result == QMessageBox::Yes)
    {
        otherThread->triggerRestart();
    }
}

void MainWindow::on_server_start_clicked()
{
    QMessageBox::StandardButton result = QMessageBox::question(this, "確認", "您確定要啟動伺服器嗎？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (result == QMessageBox::Yes)
    {
        QString executablePath = QDir::currentPath() + "/steamapps/common/PalServer/PalServer.exe";
        QProcess::startDetached(executablePath);
    }
}

void MainWindow::on_autorestart_with_time_checkBox_stateChanged(int autorestart_with_time_checkBox) {
    if (autorestart_with_time_checkBox == Qt::Checked) {
        restartTimes.clear();
        bool anyChecked = false;

        for (int i = 0; i < 24; ++i) {
            QCheckBox* checkBox = findChild<QCheckBox*>(QString("autorestart_with_time_checkBox_%1").arg(i, 2, 10, QChar('0')));
            if (checkBox && checkBox->isChecked()) {
                restartTimes.append(i);
                anyChecked = true;
            }
        }

        std::string response;
        if (rconClient.SendCommand("info", response)) {
            if (anyChecked) {
                autoRestartThread->setRestartTimes(restartTimes);
                autoRestartThread->setAutoRestartEnabled(true);
                ui->function_status->setText("已開啟自動重啟伺服器");
            } else {
                ui->function_status->setText("你沒有設定需要重啟的時間");
            }
        }
        else {
            ui->rcon_response_textBrowser->setText("您尚未連接RCON伺服器");
        }
    } else {
        autoRestartThread->setAutoRestartEnabled(false);
        ui->function_status->setText("已關閉自動重啟伺服器");
    }
}
