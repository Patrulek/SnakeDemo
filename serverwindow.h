#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QList>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QTimer>

#include "player.h"
#include "board.h"

#define FRAME_STEP 0.016

struct UdpClient {
    QHostAddress client_address;
    quint16 client_port;
};

namespace Ui {
class ServerWindow;
}

class ServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ServerWindow(QWidget *parent = 0);
    ~ServerWindow();

    void closeEvent(QCloseEvent *);

private slots:
    void addClient();
    void startServer();
    void stopServer();
    void removeClient();
    void readData();

    void readDatagrams();
    void updateGameState();

private:
    Ui::ServerWindow * ui;
    QTcpServer * server;
    QTcpSocket * clients[MAX_PLAYERS];
    QUdpSocket * udp_socket;
    UdpClient * udp_clients_info[MAX_PLAYERS];

    QTimer * timer;
    QList<int> players_to_remove;

    quint16 local_port;

    bool running;
    bool restarting;
    float restart_time;
    bool need_to_restart;
    float need_to_restart_time;
    Board * board;
    Player * players[MAX_PLAYERS];
    short players_number;


    void updateRestarting();
    void updateRunning();
    void updateNeedToRestart();
    void spawnPlayers();
};

#endif // SERVERWINDOW_H
