#include <QDateTime>
#include <QNetworkInterface>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->ui->setupUi(this);
    // 连接状态
    this->connected = false;
    // 连接了客户端的Socket
    this->socket = nullptr;
    // 有客户端连接进来
    connect(&server, SIGNAL(newConnection()), this, SLOT(server_newConnection()));
    connect(this->ui->clientsBox,SIGNAL(currentIndexChanged(int)),this,SLOT(updateSocket(int)));

    this->ui->btnConnect->setFocus();

}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief MainWindow::socket_events
 * 设置socket相关的信号槽
 */
void MainWindow::socket_events()
{
    // 当设备即将关闭时
    connect(socket, SIGNAL(aboutToClose()), this, SLOT(socket_aboutToClose()));
    // 每次将数据有效载荷写入设备的当前写入通道时
    connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(socket_bytesWritten(qint64)));
    // 断开连接时
    connect(socket, SIGNAL(disconnected()), this, SLOT(socket_disconnected()));
    // 出现错误
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socket_error()));
    // 有消息接收readyRead()
    connect(socket, SIGNAL(readyRead()), this, SLOT(socket_readyRead()));
    // 每当qabstractsocket的状态更改时，都会发出此信号。 SocketState参数是新状态。
    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socket_stateChanged()));

    // 在调用ConnectToHost（）之后发出此信号，并成功建立了连接。作为客户端连接服务器
    connect(socket, SIGNAL(connected()), this, SLOT(socket_connected()));
}

/**
 * @brief MainWindow::on_btnConnect_clicked
 * 点击连接后，客户端模式
 */
void MainWindow::on_btnConnect_clicked()
{
    if(!connected)
    {
        update(true);
        this->socket = new QTcpSocket(this);
        socket_events();
        this->socket->connectToHost(ui->txtIP->text(), ui->portNum->text().toInt());
    }
    else
    {
        update(false);
        this->server.close();
        if(this->socket != nullptr)
        {
            this->socket->disconnectFromHost();
            this->socket->deleteLater();
            this->socket = nullptr;
        }
    }
}

/**
 * @brief MainWindow::on_btnListen_clicked
 * 启动socker进行监听
 */
void MainWindow::on_btnListen_clicked()
{
    update(true);
    server.listen(QHostAddress(ui->txtIP->text()), ui->portNum->text().toInt());
}


/**
 * @brief MainWindow::update
 * @param connected
 * 监听按钮点击处理函数
 * 更新窗口显示
 */
void MainWindow::update(bool connected)
{
    this->connected = connected;
    if(connected)
    {
        ui->btnConnect->setText("断开");
        ui->btnListen->setEnabled(false);
        ui->txtIP->setEnabled(false);
        ui->portNum->setEnabled(false);
        ui->btnSend->setEnabled(true);
    }
    else
    {
        ui->btnConnect->setText("连接");
        ui->btnListen->setEnabled(true);
        ui->txtIP->setEnabled(true);
        ui->portNum->setEnabled(true);
        ui->btnSend->setEnabled(false);
    }
}

/**
 * @brief MainWindow::on_btnSend_clicked
 * 向socket中发送消息
 */
void MainWindow::on_btnSend_clicked()
{
    QString txt = ui->txtSend->toPlainText().replace('\r', "").replace('\n', "\r\n");
    socket->write(txt.toUtf8());
    log(Ui::log_type::SEND,txt);
}


/**
 * @brief MainWindow::on_btnClear_clicked
 * 清除log
 */
void MainWindow::on_btnClear_clicked()
{
    ui->txtReceive->clear();
}

void MainWindow::socket_aboutToClose()
{
    log(Ui::log_type::LOG,"socket_aboutToClose");
}

void MainWindow::socket_bytesWritten(qint64 bytes)
{
    bytes;
//    log(Ui::log_type::SEND,QString::fill(bytes));
}

void MainWindow::socket_connected()
{
    log(Ui::log_type::LOG,"连接成功");
}

void MainWindow::socket_disconnected()
{
    log(Ui::log_type::LOG,"断开连接");
    update(false);
}

void MainWindow::socket_error()
{
    log(Ui::log_type::LOG,socket->errorString());
}

void MainWindow::socket_readyRead()
{
    while(socket->bytesAvailable())
    {
        QByteArray data = socket->readAll();
        log(Ui::log_type::RECEIVE,data);
    }
}

void MainWindow::socket_stateChanged()
{
//    log("socket_stateChanged");
}

void MainWindow::server_newConnection()
{

    QTcpSocket *socket = server.nextPendingConnection();
    this->socket = socket;
    socket_events();
//    server.close();// 关闭listener,但已经连接的还能发送消息
    QString ip = socket->peerAddress().toString();
    qint16 port = socket->peerPort();
    this->clients.append(socket);
    this->ui->clientsBox->addItem(QString("[%1:%2] ").arg(ip).arg(port));
    this->log(Ui::log_type::LOG,"连接到server");
}

void MainWindow::log(enum Ui::log_type type,QString msg)
{

    QDateTime dateTime= QDateTime::currentDateTime();
    QString ip = this->socket->peerAddress().toString();
    qint16 port = this->socket->peerPort();
    QString tmp = dateTime.toString(" yyyy-MM-dd hh:mm:ss ")+QString("[%1:%2] ").arg(ip).arg(port);
    switch(type)
    {
        case Ui::log_type::LOG:
            ui->txtReceive->appendPlainText("[log]"+tmp+msg);
        break;
        case Ui::log_type::RECEIVE:
            ui->txtReceive->appendPlainText("[rev]"+tmp+msg);
        break;
        case Ui::log_type::SEND:
            ui->txtReceive->appendPlainText("[sen]"+tmp+msg);
        break;
        default:
            ui->txtReceive->appendPlainText("[dfu]"+tmp+msg);
    }

}

void MainWindow::updateSocket(int index){
    qDebug()<<index;
    this->socket = this->clients[index];
}
