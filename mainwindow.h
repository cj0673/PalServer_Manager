#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "RCONClient.h"
#include "AutoRestartThread.h"
#include "OtherThread.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connect_rcon_pushButton_clicked();
    void on_connect_rcon_command_info_clicked();
    void on_connect_rcon_command_showplayers_clicked();
    void on_connect_rcon_command_save_clicked();
    void on_connect_rcon_command_shutdown_clicked();
    void on_connect_rcon_command_broadcast_clicked();
    void on_server_restart_clicked();
    void on_server_start_clicked();
    void on_autorestart_with_time_checkBox_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
    RCONClient rconClient;
    AutoRestartThread *autoRestartThread;
    OtherThread *otherThread;
    QList<int> restartTimes;
};
#endif // MAINWINDOW_H
