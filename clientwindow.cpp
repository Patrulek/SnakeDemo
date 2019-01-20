#include "clientwindow.h"

#include "ui_clientjoinwindow.h"
#include "ui_clientgameplaywindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QTime>

#include <string>
#include <cmath>

#include "player.h"
#include "message.h"


ClientWindow::ClientWindow(QWidget * parent) :
    QMainWindow(parent),
    ui_join(new Ui::ClientJoinWindow)
{
    qsrand(QTime::currentTime().msec());

    ui_join->setupUi(this);
    ui_gameplay = 0;//nullptr;

    tcp_socket = new QTcpSocket(this);
    udp_socket = new QUdpSocket(this);

    connect(tcp_socket, SIGNAL(aboutToClose()), this, SLOT(cannotConnect()));
    connect(tcp_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(tcp_socket, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(ui_join->btn_connect, SIGNAL(clicked(bool)), this, SLOT(connectToServer()));

    for(int i = 0; i < MAX_PLAYERS; i++)
        players[i] = 0;//nullptr;

    timer = new QTimer(this);
    timer->start(16);
}


ClientWindow::~ClientWindow()
{
    tcp_socket->deleteLater();
    udp_socket->deleteLater();

    delete board;

    for( int i = 0; i < MAX_PLAYERS; i++ )
        if( players[i ])
            delete players[i];

    if( ui_gameplay )
        delete ui_gameplay;

    if( ui_join )
        delete ui_join;
}

void ClientWindow::closeEvent(QCloseEvent *) {
    QApplication::exit(0);
}


void ClientWindow::updateGameState()
{
    setFocus(Qt::NoFocusReason);
    Player::Action act = players[0]->getAction();
    sendDatagram(udp_socket, server_address, server_udp_port, MSG_SEND_ACTION, sizeof(act), (const char *)&act);
}

void ClientWindow::updatePlayersPanel() {
    int pix_w = ui_gameplay->lab_color1->width();
    int pix_h = ui_gameplay->lab_color1->height();
    QPixmap pix1(pix_w, pix_h);
    QPixmap pix2(pix_w, pix_h);
    QPixmap pix3(pix_w, pix_h);
    QPixmap pix4(pix_w, pix_h);

    if( players[0] ) {
        ui_gameplay->lab_nick1->setText(players[0]->getName());
        ui_gameplay->lab_score1->setText(tr("Score: ") + QString::number(players[0]->getScore()));
        pix1.fill(players[0]->getColor());
        ui_gameplay->lab_color1->setPixmap(pix1);
    }

    if( players[1] ) {
        ui_gameplay->lab_nick2->setText(players[1]->getName());
        ui_gameplay->lab_score2->setText(tr("Score: ") + QString::number(players[1]->getScore()));
        pix2.fill(players[1]->getColor());
        ui_gameplay->lab_color2->setPixmap(pix2);
    } else {
        pix2.fill(Qt::black);
        ui_gameplay->lab_nick2->setText(tr("No player"));
        ui_gameplay->lab_score2->setText(tr("Score: -"));
        ui_gameplay->lab_color2->setPixmap(pix2);
    }

    if( players[2] ) {
        ui_gameplay->lab_nick3->setText(players[2]->getName());
        ui_gameplay->lab_score3->setText(tr("Score: ") + QString::number(players[2]->getScore()));
        pix3.fill(players[2]->getColor());
        ui_gameplay->lab_color3->setPixmap(pix3);
    } else {
        pix3.fill(Qt::black);
        ui_gameplay->lab_nick3->setText(tr("No player"));
        ui_gameplay->lab_score3->setText(tr("Score: -"));
        ui_gameplay->lab_color3->setPixmap(pix3);
    }

    if( players[3] ) {
        ui_gameplay->lab_nick4->setText(players[3]->getName());
        ui_gameplay->lab_score4->setText(tr("Score: ") + QString::number(players[3]->getScore()));
        pix4.fill(players[3]->getColor());
        ui_gameplay->lab_color4->setPixmap(pix4);
    } else {
        pix4.fill(Qt::black);
        ui_gameplay->lab_nick4->setText(tr("No player"));
        ui_gameplay->lab_score4->setText(tr("Score: -"));
        ui_gameplay->lab_color4->setPixmap(pix4);
    }
}

void ClientWindow::keyPressEvent(QKeyEvent * event) {
    if( event->key() == Qt::Key_Left )
        players[0]->setAction(Player::ACTION_LEFT);
    else if( event->key() == Qt::Key_Right )
        players[0]->setAction(Player::ACTION_RIGHT);
}

void ClientWindow::keyReleaseEvent(QKeyEvent * event) {
    if( event->key() == Qt::Key_Left && players[0]->getAction() != Player::ACTION_RIGHT )
        players[0]->setAction(Player::NO_ACTION);
    else if( event->key() == Qt::Key_Right && players[0]->getAction() != Player::ACTION_LEFT )
        players[0]->setAction(Player::NO_ACTION);
}

void ClientWindow::sendReadiness() {
    bool readiness = false;

    if( !players[0]->getReady() ) {
        players[0]->setReady(true);
        readiness = true;
        ui_gameplay->btn_readiness->setText(tr("Not ready"));
    } else {
        players[0]->setReady(false);
        ui_gameplay->btn_readiness->setText(tr("Ready"));
    }

    sendMessage(tcp_socket, MSG_SEND_READINESS, 1, (const char *)&readiness);
}

void ClientWindow::startGame() {
    connect(timer, SIGNAL(timeout()), this, SLOT(updateGameState()));
}


void ClientWindow::stopGame() {
    disconnect(timer, SIGNAL(timeout()), this, SLOT(updateGameState()));
}


void ClientWindow::changeUI() {
    this->setGeometry((SCREEN_X - WINDOW_X) / 2, (SCREEN_Y - WINDOW_Y) / 2, WINDOW_X, WINDOW_Y);
    ui_gameplay = new Ui::ClientGameplayWindow();
    ui_gameplay->setupUi(this);

    board = new Board(ui_gameplay->gv_viewport, players);
    connect(ui_gameplay->btn_readiness, SIGNAL(clicked(bool)), this, SLOT(sendReadiness()));
    connect(ui_gameplay->btn_disconnect, SIGNAL(clicked(bool)), this, SLOT(disconnectFromServer()));
}

void ClientWindow::disconnectFromServer() {
    tcp_socket->disconnectFromHost();
    QApplication::exit(0);
}

void ClientWindow::cannotConnect() {
    QMessageBox::information(this, tr("Error!"), tr("Server busy, try again later"));
    tcp_socket->disconnectFromHost();
}

void ClientWindow::connectToServer() {
    if( tcp_socket->state() != QAbstractSocket::ConnectedState && tcp_socket->state() != QAbstractSocket::ConnectingState ) {
        tcp_socket->abort();
        QString ip_address = ui_join->le_ip_address->text();
        quint16 port = ui_join->le_port->text().toInt();
        tcp_socket->connectToHost(ip_address, port);
    }
}

void ClientWindow::readDatagrams() {
    Msg msg;

    while( udp_socket->hasPendingDatagrams() ) {
       // QHostAddress address;
       // quint16 port;

      //  client->readDatagram((char *)&msg, client->pendingDatagramSize(), &address, &port);
        udp_socket->readDatagram((char *)&msg, udp_socket->pendingDatagramSize());

        if( msg.header.msg_type == MSG_UPDATE_PLAYER ) {
            int ptr = 0;
            int players_number = msg.header.block_size / 9;

            for( int i = 0; i < players_number; i++ ) {
                if( players[i] ) {
                    bool * is_alive;
                    float position[2];
                    ((char *)is_alive)[0] = msg.data[ptr++];

                    for(int j = 0; j < sizeof(position); j++)
                        ((char *)position)[j] = msg.data[ptr++];

                    if( !(*is_alive) ) {
                        players[i]->setAlive(false);

                        if( i == 0 )
                            stopGame();
                    }

                    players[i]->setPosition(position);
                    players[i]->setX((int)position[0]);
                    players[i]->setY((int)position[1]);
                }
            }
            board->updateImage();
        }

     //   qDebug() << "datagram od: " << index;
      /*  qDebug() << "address: " << client->localAddress() << " | " << address;
        qDebug() << "port: " << client->localPort() << " | " << port; */
    }
}

void ClientWindow::readData() {
    while( tcp_socket->bytesAvailable() >= HEADER_SIZE ) {
        Msg msg;
        tcp_socket->read((char *)&msg.header, HEADER_SIZE);

        if( msg.header.msg_type == MSG_JOIN_TO_SERVER ) {
            tcp_socket->read(msg.data, msg.header.block_size);
            bool * can_connect = (bool *)msg.data;

            if( !can_connect ) {
                cannotConnect();
                return;
            } else {
                QString str(ui_join->le_nickname->text());
                QByteArray strArr = str.toLocal8Bit();
                char * nickname = strArr.data();

                udp_socket->bind(tcp_socket->localAddress());
                connect(udp_socket, SIGNAL(readyRead()), this, SLOT(readDatagrams()));
                local_port = udp_socket->localPort();

                sendMessage(tcp_socket, MSG_JOIN_TO_SERVER, 32, (const char *)nickname);
                sendMessage(tcp_socket, MSG_SEND_UDP_PORT, 2, (const char *)&local_port);

                changeUI();
            }
        } else if( msg.header.msg_type == MSG_SEND_UDP_PORT ) {
            tcp_socket->read(msg.data, msg.header.block_size);
            quint16 * server_port = (quint16 *)msg.data;
            server_udp_port = *server_port;
            server_address = tcp_socket->peerAddress();

     //       qDebug() << "Local port (receive datagram): " << local_port;
     //       qDebug() << "Local address (receive datagram): " << tcp_socket->localAddress();
     //       qDebug() << "Server port (send datagram): " << server_udp_port;
     //       qDebug() << "Server address (send datagram): " << server_address;
        } else if( msg.header.msg_type == MSG_NEW_PLAYER ) {
            tcp_socket->read(msg.data, msg.header.block_size);
            PlayerData * p_data = (PlayerData *)msg.data;
            int index = 0;

            for( int i = 0; i < MAX_PLAYERS; i++ )
                if( !players[i] ) {
                    index = i;
                    break;
                }

            players[index] = new Player(*p_data);
            board->setPlayer(players[index], index);
            updatePlayersPanel();
        } else if( msg.header.msg_type == MSG_GAME_STARTED ) {
            startGame();
            ui_gameplay->lab_counter->setEnabled(false);
            ui_gameplay->btn_readiness->setEnabled(false);
        } else if( msg.header.msg_type == MSG_PLAYER_LEFT ) {
            tcp_socket->read(msg.data, msg.header.block_size);

            for( int i = 0; i < MAX_PLAYERS; i++ ) {
                if( players[i] ) {
                    players[i]->setReady(false);
                    ui_gameplay->btn_readiness->setText(tr("Ready"));

                    if( players[i]->getName() == tr(msg.data) ) {
                        delete players[i];
                        players[i] = 0;//nullptr;
                    }
                }
            }

            updatePlayersPanel();
        } else if( msg.header.msg_type == MSG_GAME_ENDED ) {
            stopGame();
            ui_gameplay->lab_counter->setEnabled(false);
            ui_gameplay->btn_readiness->setEnabled(true);
            ui_gameplay->btn_readiness->setText(tr("Ready"));
            tcp_socket->read(msg.data, msg.header.block_size);

            for( int i = 0; i < MAX_PLAYERS; i++ ) {
                if( players[i] )
                    players[i]->clearStats();
            }

            QMessageBox::information(this, tr("Game over!"), tr("The winner is ") + tr(msg.data) + tr("\nCongratulations!"));
            updatePlayersPanel();
        } else if( msg.header.msg_type == MSG_SERVER_DOWN ) {
          tcp_socket->disconnectFromHost();
          QMessageBox::information(this, tr("Server down!"), tr("Server down!"));
          QApplication::quit();
        } else if( msg.header.msg_type == MSG_RESPAWN_PLAYER ) {
            for( int i = 0; i < MAX_PLAYERS; i++ ) {
                if( !players[i] )
                    continue;

                players[i]->setAlive(true);
                tcp_socket->read(msg.data, 8);
                float * pos = (float *)msg.data;
                players[i]->setPosition(pos);
                players[i]->setX((int)pos[0]);
                players[i]->setY((int)pos[1]);
                tcp_socket->read(msg.data, 2);
                short * score = (short *)msg.data;
                players[i]->setScore(*score);
            }

            ui_gameplay->lab_counter->setText(tr("3"));
            ui_gameplay->lab_counter->setEnabled(true);
            stopGame();
            updatePlayersPanel();
            board->clearBoard();
            board->updateImage();
        } else if( msg.header.msg_type == MSG_RESTART_GAME ) {
            tcp_socket->read(msg.data, msg.header.block_size);
            float * time = (float *)msg.data;
            ui_gameplay->lab_counter->setText(QString::number(std::ceil(*time)));
        }
    }
}

void ClientWindow::connectionError(QAbstractSocket::SocketError socketError) {
    switch( socketError ) {
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
                                             .arg(tcp_socket->errorString()));
    }
}
