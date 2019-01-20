#ifndef CLIENTJOINWINDOW_H
#define CLIENTJOINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

namespace Ui {
class ClientJoinWindow;
}

class ClientJoinWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClientJoinWindow(QWidget *parent = 0);
    ~ClientJoinWindow();

    void closeEvent(QCloseEvent *);

private slots:
    void connectedToServer();
    void connectToServer();
    void connectionError(QAbstractSocket::SocketError);
    void clientNotConnected();
    void handleStateChanged(QAbstractSocket::SocketState);

private:
    Ui::ClientJoinWindow *ui;
    QTcpSocket * tcpSocket;
    quint16 blockSize;
};

#endif // CLIENTJOINWINDOW_H
