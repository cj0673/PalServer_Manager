#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "RCONClient.h"

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

private:
    Ui::MainWindow *ui;

    RCONClient rconClient;
};
#endif // MAINWINDOW_H
