#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QUdpSocket>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QImage>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QApplication>
#include <QDesktopWidget>

#include "player.h"
#include "board.h"

#define FRAME_STEP 0.016
#define SCREEN_X (QApplication::desktop()->screenGeometry().width())
#define SCREEN_Y (QApplication::desktop()->screenGeometry().height())
#define WINDOW_X 800
#define WINDOW_Y 600

namespace Ui {
class ClientJoinWindow;
class ClientGameplayWindow;
}

class ClientWindow : public QMainWindow
{
    Q_OBJECT





public:
    explicit ClientWindow(QWidget *parent = 0);
    ~ClientWindow();

    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);


private slots:
    void readData();
    void connectToServer();
    void disconnectFromServer();
    void connectionError(QAbstractSocket::SocketError);
    void cannotConnect();

    void readDatagrams();
    void sendReadiness();
    void startGame();
    void stopGame();
    void updateGameState();

private:
    Ui::ClientGameplayWindow * ui_gameplay;
    Ui::ClientJoinWindow * ui_join;
    QTcpSocket * tcp_socket;
    QUdpSocket * udp_socket;

    QHostAddress server_address;
    quint16 server_udp_port;
    quint16 local_port;

    QTimer * timer;
    Player * players[MAX_PLAYERS];

    Board * board;

    void updatePlayersPanel();
    void changeUI();
};

#endif // CLIENTWINDOW_H
