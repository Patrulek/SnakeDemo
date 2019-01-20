#include "clientjoinwindow.h"
#include "ui_clientjoinwindow.h"
#include "ui_clientgameplaywindow.h"

#include "clientgameplaywindow.h"


#include <QMessageBox>
#include <QDebug>

ClientJoinWindow::ClientJoinWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClientJoinWindow)
{
    ui->setupUi(this);

    tcpSocket = new QTcpSocket(this);

    qDebug() << tcpSocket;
    tcpSocket->setProperty("successful", QVariant(false));

    connect(tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(handleStateChanged(QAbstractSocket::SocketState)));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(connectedToServer()));
    connect(tcpSocket, SIGNAL(aboutToClose()), this, SLOT(clientNotConnected()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));

    connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(connectToServer()));
}

ClientJoinWindow::~ClientJoinWindow()
{
    delete ui;
}

void ClientJoinWindow::handleStateChanged(QAbstractSocket::SocketState state) {
    switch(state) {
        case QAbstractSocket::ConnectedState:
            ClientGameplayWindow * w = new ClientGameplayWindow(tcpSocket, blockSize);
            w->show();
            this->close();
            break;
    }
}

void ClientJoinWindow::closeEvent(QCloseEvent * event) {
   // tcpSocket->disconnectFromHost();
    //this->hide();
}

void ClientJoinWindow::clientNotConnected() {
    QMessageBox::information(this, tr("Brak miejsc"), tr("Server zajęty, spróbuj później"));
}

void ClientJoinWindow::connectToServer() {
    if(tcpSocket->state() != QAbstractSocket::ConnectedState && tcpSocket->state() != QAbstractSocket::ConnectingState) {
        tcpSocket->abort();
        blockSize = 0;

        QString ipAddress;
        quint16 port;

        ipAddress = ui->lineEdit->text();
        port = ui->lineEdit_2->text().toInt();

        tcpSocket->connectToHost(ipAddress, port);
    }
}

void ClientJoinWindow::connectedToServer() {
}

void ClientJoinWindow::connectionError(QAbstractSocket::SocketError socketError) {
    switch (socketError)
        {
        case QAbstractSocket::RemoteHostClosedError:
            break;

        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, tr("Fortune Client"),
                                             tr("The host was not found. Please check the "
                                                 "host name and port settings."));
            break;

        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, tr("Fortune Client"),
                                             tr("The connection was refused by the peer. "
                                                 "Make sure the fortune server is running, "
                                                 "and check that the host name and port "
                                                 "settings are correct."));
            break;

        default:
            QMessageBox::information(this, tr("Fortune Client"),
                                             tr("The following error occurred: %1.")
                                             .arg(tcpSocket->errorString()));
        }
}
