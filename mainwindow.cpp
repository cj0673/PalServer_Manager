#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    QString tempPath = QDir::tempPath();
    QString filePath = tempPath + QDir::separator() + "settings.txt";

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString strIp, strPort, strPassword;
        if (!in.atEnd()) {
            strIp = in.readLine();
        }
        if (!in.atEnd()) {
            strPort = in.readLine();
        }
        if (!in.atEnd()) {
            strPassword = in.readLine();
        }
        file.close();

        ui->rcon_ip_lineEdit->setText(strIp);
        ui->rcon_port_lineEdit->setText(strPort);
        ui->rcon_password_lineEdit->setText(strPassword);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_connect_rcon_pushButton_clicked()
{
    QString ip = ui->rcon_ip_lineEdit->text();
    int port = ui->rcon_port_lineEdit->text().toInt();
    QString password = ui->rcon_password_lineEdit->text();

    std::string ip_str = ip.toStdString();
    std::string password_str = password.toStdString();

    if (rconClient.Connect(ip_str, port, password_str)) {
        QString tempPath = QDir::tempPath();
        QString filePath = tempPath + QDir::separator() + "settings.txt";
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);

            out << ip << "\n";
            out << port << "\n";
            out << password << "\n";

            file.close();
        }

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
        size_t newlinePos = response.find('\n');
        response.replace(0, newlinePos, "資料排序為：名稱, playeruid, steamid");
        ui->rcon_response_textBrowser->setText(QString(response.c_str()));
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
        if (rconClient.SendCommand("shutdown 1", response)) {
            ui->rcon_response_textBrowser->setText(QString(response.c_str()));
        }
        else {
            ui->connect_rcon_pushButton->setDisabled(false);
            ui->rcon_response_textBrowser->setText("無法發送指令或接收回應");
        }
    }
}
