#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

namespace Ui {
class MainWindow;
enum log_type {SEND,RECEIVE,LOG};
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_btnConnect_clicked();
    void on_btnListen_clicked();
    void on_btnSend_clicked();
    void on_btnClear_clicked();
    void socket_aboutToClose();
    void socket_bytesWritten(qint64 bytes);
    void socket_connected();
    void socket_disconnected();
    void socket_error();
    void socket_readyRead();
    void socket_stateChanged();
    void server_newConnection();
    void updateSocket(int index);

private:
    Ui::MainWindow *ui;
    bool connected;
    QTcpServer server;
    QTcpSocket *socket;
    QList<QTcpSocket *> clients;

private:
    void log(enum Ui::log_type type,QString msg);
    void socket_events();
    void update(bool connected);
};

#endif // MAINWINDOW_H
