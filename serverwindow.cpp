#include "serverwindow.h"
#include "ui_serverwindow.h"

//#include <QNetworkInterface>
#include <QMessageBox>
#include <QCloseEvent>
#include <QHostAddress>

#include <QDebug>
#include <QTime>

#include "message.h"
#include <cmath>

ServerWindow::ServerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ServerWindow),
    players_number(0)
{
    qsrand(QTime::currentTime().msec());
    running = restarting = need_to_restart = false;

    timer = new QTimer(this);
    timer->start(16);

    ui->setupUi(this);

    ui->le_port->setValidator(new QIntValidator(1024, 65535, this));

    server = new QTcpServer(this);
    server->setMaxPendingConnections(MAX_PLAYERS);

    connect(server, SIGNAL(newConnection()), this, SLOT(addClient()));
    connect(ui->btn_start_server, SIGNAL(clicked(bool)), this, SLOT(startServer()));
    connect(ui->btn_stop_server, SIGNAL(clicked(bool)), this, SLOT(stopServer()));

    for( int i = 0; i < MAX_PLAYERS; i++ ) {
        players[i] = 0;//nullptr;
        clients[i] = 0;//nullptr;
        udp_clients_info[i] = 0;//nullptr;
    }

    udp_socket = 0;//nullptr;
    board = new Board();
}

ServerWindow::~ServerWindow()
{
    server->close();

    delete ui;
    delete board;

    for( int i = 0; i < MAX_PLAYERS; i++ ) {
        if( players[i] )
            delete players[i];

        if( clients[i] )
            clients[i]->deleteLater();
    }

    udp_socket->deleteLater();
    server->deleteLater();
}

void ServerWindow::addClient()
{
    bool can_add = true;
    QTcpSocket * client = server->nextPendingConnection();

    if( players_number >= MAX_PLAYERS || running || restarting || need_to_restart )
        can_add = false;

    sendMessage(client, MSG_JOIN_TO_SERVER, 1, (const char *)&can_add);

    if( !can_add )
        return;

    for( int i = 0; i < MAX_PLAYERS; i++ )
        if( !clients[i] ) {
            clients[i] = client;

            if( !udp_socket ) {
                udp_socket = new QUdpSocket(this);
                udp_socket->bind(client->localAddress());
                local_port = udp_socket->localPort();
                connect(udp_socket, SIGNAL(readyRead()), this, SLOT(readDatagrams()));
            }

            udp_clients_info[i] = new UdpClient();
            udp_clients_info[i]->client_address = client->peerAddress();
            sendMessage(client, MSG_SEND_UDP_PORT, sizeof(local_port), (const char *)&local_port);

            break;
        }

    connect(client, SIGNAL(disconnected()), this, SLOT(removeClient()));
    connect(client, SIGNAL(readyRead()), this, SLOT(readData()));
}

void ServerWindow::readDatagrams() {
    while( udp_socket->hasPendingDatagrams() ) {
        Msg msg;
        QHostAddress address;
        quint16 port;
        udp_socket->readDatagram((char *)&msg, udp_socket->pendingDatagramSize(), &address, &port);
        int index = -1;

        for( int i = 0; i < MAX_PLAYERS; i++ )
            if( udp_clients_info[i] && address == udp_clients_info[i]->client_address && port == udp_clients_info[i]->client_port )
                index = i;

        if( msg.header.msg_type == MSG_SEND_ACTION && index != -1 ) {
            Player::Action * action = (Player::Action *)msg.data;
            players[index]->setAction(*action);
        }
    }
}

void ServerWindow::closeEvent(QCloseEvent * event) {
    QApplication::exit(0);
}

void ServerWindow::stopServer() {
    server->close();
    ui->lab_server_status->setText("Down");
    ui->btn_stop_server->setEnabled(false);
    ui->btn_start_server->setEnabled(true);
    ui->le_port->setEnabled(true);
    running = restarting = need_to_restart = false;

    for( int i = 0; i < MAX_PLAYERS; i++ ) {
        if( clients[i] )
            sendMessage(clients[i], MSG_SERVER_DOWN, 0, 0/*nullptr*/);
    }
}

void ServerWindow::removeClient()
{
    QTcpSocket * client = (QTcpSocket *)sender();
    int index = -1;

    for( int i = 0; i < MAX_PLAYERS; i++ )
        if( clients[i] == client ) {
            index = i;
            break;
        }

    QByteArray arr = players[index]->getName().toLocal8Bit();
    char * nickname = arr.data();

    for(int i = 0; i < MAX_PLAYERS; i++) {
        QTcpSocket * cl = clients[i];

        if( !(running || restarting || need_to_restart) ) {
            if( cl ) {
                players[i]->setReady(false);
                sendMessage(cl, MSG_PLAYER_LEFT, 32, (const char *)nickname);
            }
        } else {
            if( !players_to_remove.contains(index) )
                players_to_remove.append(index);
        }

    }

    if( !players_to_remove.contains(index) ) {
        delete players[index];
        players[index] = 0;//nullptr;
        players_number--;
        clients[index]->deleteLater();
        clients[index] = 0;//nullptr;
    }

    ui->lab_players->setText(QString("Players: " + QString::number(players_number)));
}

void ServerWindow::readData() {
    QTcpSocket * client = (QTcpSocket *)sender();
    int index = -1;

    for( int i = 0; i < MAX_PLAYERS; i++ )
        if( client == clients[i] ) {
            index = i;
            break;
        }

    while( client->bytesAvailable() >= HEADER_SIZE ) {
        Msg msg;

        if( index > -1 && index < MAX_PLAYERS ) {
            client->read((char *)&msg, HEADER_SIZE);

            if( msg.header.msg_type == MSG_SEND_READINESS ) {
                client->read(msg.data, msg.header.block_size);
                bool * readiness = (bool *)msg.data;

                if( *readiness ) {
                    players[index]->setReady(true);
                    int cnt = 0;

                    for( int i = 0; i < MAX_PLAYERS; i++ ) {
                        if( players[i] && players[i]->getReady() )
                            cnt++;

                        if( cnt == players_number && cnt > 1 ) {
                            need_to_restart = true;
                            need_to_restart_time = 1.5f;

                            connect(timer, SIGNAL(timeout()), this, SLOT(updateGameState()));
                            break;
                        }
                    }
                } else {
                    players[index]->setReady(false);

                    if( restarting || need_to_restart ) {
                        restarting = need_to_restart = false;
                        disconnect(timer, SIGNAL(timeout()), this, SLOT(updateGameState()));
                    }
                }
            } else if( msg.header.msg_type == MSG_SEND_UDP_PORT ) {
                client->read(msg.data, msg.header.block_size);

                quint16 * client_udp_port = (quint16 *)msg.data;
                udp_clients_info[index]->client_port = *client_udp_port;

      //          qDebug() << "Local port (receive datagram): " << local_port;
      //          qDebug() << "Local address (receive datagram): " << client->localAddress();
      //          qDebug() << "Client port (send datagram): " << udp_clients_info[index]->client_port;
      //          qDebug() << "Client address (send datagram): " << udp_clients_info[index]->client_address;

            } else if( msg.header.msg_type == MSG_JOIN_TO_SERVER ) { // wyslano nick gracza
                client->read(msg.data, msg.header.block_size);
                QString str((const char *)msg.data);
                int index = 0;

                for( int i = 0; i < MAX_PLAYERS; i++ )
                    if(!players[i]) {
                        index = i;
                        players_number++;
                        break;
                    }

                players[index] = new Player();
                players[index]->setName(str);
                board->setPlayer(players[index], index);
                PlayerData player_data(players[index]->getColor(), players[index]->getPosition(),
                        players[index]->getAngle(), players[index]->getName());

                for( int i = 0; i < MAX_PLAYERS; i++ ) {
                    QTcpSocket * cl = clients[i];

                    if( cl )
                        sendMessage(cl, MSG_NEW_PLAYER, sizeof(player_data), (const char *)&player_data);
                }

                for( int i = 0; i < MAX_PLAYERS; i++ ) {
                    if( i == index )
                        continue;

                    if( players[i] ) {
                        PlayerData pl_data(players[i]->getColor(), players[i]->getPosition(), players[i]->getAngle(),
                                           players[i]->getName());
                        sendMessage(client, MSG_NEW_PLAYER, sizeof(pl_data), (const char *)&pl_data);
                    }
                }

                ui->lab_players->setText(QString("Players: " + QString::number(players_number)));
            }
        }
    }
}

void ServerWindow::updateRestarting() {
    int last_ceil = std::ceil(restart_time);
    restart_time -= FRAME_STEP;

    if( ceil(restart_time) != last_ceil ) {
        for( int i = 0; i < MAX_PLAYERS; i++ ) {
            QTcpSocket * client = clients[i];

            if( client )
                sendMessage(client, MSG_RESTART_GAME, sizeof(float), (const char *)&restart_time);
        }
    }

    if( restart_time < 0 ) {
        restart_time = 0.0f;
        restarting = false;
        running = true;

        for( int i = 0; i < MAX_PLAYERS; i++ ) {
            QTcpSocket * client = clients[i];

            if( client )
                sendMessage(client, MSG_GAME_STARTED, 0, 0/*nullptr*/);
        }
    }
}

void ServerWindow::updateRunning() {
    bool wasAlive[] = {0, 0, 0, 0};

    for( int i = 0; i < MAX_PLAYERS; i++ ) {
        if( players[i] && players[i]->isAlive() ) {
            players[i]->update(FRAME_STEP);
            wasAlive[i] = true;
        }
    }

    board->checkCollision();
    short alives = 0;

    for( int i = 0; i < MAX_PLAYERS; i++ ) {
        if( !players[i] )
            continue;

        QTcpSocket * client = clients[i];

        if( client ) {
            DataToSend new_data;
            bool is_alive = players[i]->isAlive();

            if( is_alive )
                alives++;

            new_data.addData((char *)&is_alive, 1);
            new_data.addData((char *)players[i]->getPosition(), 8);

            for( int j = 0; j < MAX_PLAYERS; j++ ) {
                if( i == j || !players[j] )
                     continue;

                is_alive = players[j]->isAlive();
                new_data.addData((char *)&is_alive, 1);
                new_data.addData((char *)players[j]->getPosition(), 8);
            }

            sendDatagram(udp_socket, udp_clients_info[i]->client_address, udp_clients_info[i]->client_port,
                         MSG_UPDATE_PLAYER, 9 * players_number, (const char *)new_data.data);
        }
    }

    if( alives < 2 ) {
        need_to_restart = true;
        need_to_restart_time = 1.5f;
        running = false;

        for( int i = 0; i < MAX_PLAYERS; i++ ) {
            if( players[i] && players[i]->isAlive() ) {
                players[i]->setScore(players[i]->getScore() + 1);

                if( players[i]->getScore() == 3 ) {
                    QByteArray strArr = players[i]->getName().toLocal8Bit();
                    char * nickname = strArr.data();

                    for( int j = 0; j < MAX_PLAYERS; j++ ) {
                        QTcpSocket * client = clients[j];

                        if( client ) {
                            players[j]->clearStats();
                            sendMessage(client, MSG_GAME_ENDED, 32, (const char *)nickname);
                        }
                    }

                    for( int j = 0; j < players_to_remove.size(); j++ ) {
                        int index = players_to_remove.at(j);

                        if( clients[index] ) {
                            strArr = players[index]->getName().toLocal8Bit();
                            nickname = strArr.data();

                            for( int k = 0; k < MAX_PLAYERS; k++ ) {
                                QTcpSocket * cl = clients[k];

                                if( cl )
                                    sendMessage(cl, MSG_PLAYER_LEFT, 32, (const char *)nickname);
                            }

                            delete players[index];
                            players[index] = 0;//nullptr;
                            clients[index]->deleteLater();
                            clients[index] = 0;//nullptr;
                            players_number--;
                            ui->lab_players->setText(tr("Players: ") + QString::number(players_number));
                        }
                    }

                    players_to_remove.clear();
                    running = false;
                    restarting = false;
                    need_to_restart = false;
                    disconnect(timer, SIGNAL(timeout()), this, SLOT(updateGameState()));
                }
                break;
            }
        }
    }
}

void ServerWindow::updateNeedToRestart() {
    need_to_restart_time -= FRAME_STEP;

    if( need_to_restart_time < 0 ) {
        spawnPlayers();
        board->clearBoard();
        restarting = true;
        need_to_restart = false;
        restart_time = 3.0f;
    }
}

void ServerWindow::updateGameState() {
    if( restarting )
        updateRestarting();
    else if( running )
        updateRunning();
    else if( need_to_restart )
        updateNeedToRestart();
}

void ServerWindow::spawnPlayers() {
    for( int i = 0; i < MAX_PLAYERS; i++ ) {
        if( players[i] )
            players[i]->spawn();
    }

    for( int i = 0; i < MAX_PLAYERS; i++ ) {
        QTcpSocket * client = clients[i];
        DataToSend new_data;

        if( client ) {
            short score = players[i]->getScore();
            new_data.addData((char *)players[i]->getPosition(), 8);
            new_data.addData((char *)&score, 2);

            for( int j = 0; j < MAX_PLAYERS; j++ ) {
                if( i == j || !players[j] )
                     continue;

                score = players[j]->getScore();
                new_data.addData((char *)players[j]->getPosition(), 8);
                new_data.addData((char *)&score, 2);
            }

            sendMessage(client, MSG_RESPAWN_PLAYER, 10 * players_number, (const char *)new_data.data);
        }

    }
}

void ServerWindow::startServer()
{
    if( ui->le_port->text().toInt() < 1024 ) {
        QMessageBox::critical(this, tr("Error!"), tr("You have to insert port number between 1024 and 65535"));
        return;
    }

    QString ip_address;
    QTcpSocket testSocket;
    testSocket.connectToHost("8.8.8.8", 53);

    if( testSocket.waitForConnected(2000) )
        ip_address = testSocket.localAddress().toString();

    if( ip_address.isEmpty() )
        ip_address = "127.0.0.1";

    if( !server->listen(QHostAddress(ip_address), ui->le_port->text().toInt()) ) {
        QMessageBox::critical(this, tr("Error!"), tr("Cannot start server!"));
        return;
    } else {
        ui->lab_server_status->setText("Up: " + ip_address + ":" + ui->le_port->text());
        ui->le_port->setEnabled(false);
        ui->btn_stop_server->setEnabled(true);
        ui->btn_start_server->setEnabled(false);
    }
}
