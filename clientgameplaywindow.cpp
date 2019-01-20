#include "clientgameplaywindow.h"
#include "ui_clientgameplaywindow.h"

#include <QDebug>
#include <QHostAddress>
#include <QTime>
#include <cmath>

ClientGameplayWindow::ClientGameplayWindow(QTcpSocket * tcpSocket, quint16 blockSize, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClientGameplayWindow),
    blockSize(blockSize)
{
    qsrand(QTime::currentTime().msec());

    ui->setupUi(this);
    players[0] = new Player();
    players[1] = nullptr;
    players[2] = nullptr;
    players[3] = nullptr;
    updatePlayersPanel();

    this->tcpSocket = tcpSocket;

    board = new Board(ui->graphicsView, players);

    running = false;

    timer = new QTimer(this);
    timer->start(16);

    board->updateImage();


    connect(ui->pushButton_2, SIGNAL(clicked(bool)), this, SLOT(startGame()));
}

ClientGameplayWindow::~ClientGameplayWindow()
{
    delete ui;
    delete tcpSocket;
    delete [] players;
}


void ClientGameplayWindow::closeEvent(QCloseEvent * event) {
     stopGame();
     QApplication::exit();
}

void ClientGameplayWindow::updateGameState()
{
    setFocus(Qt::NoFocusReason);

    for(int i = 0; i < MAX_PLAYERS; i++) {
        if(players[i] && players[i]->isAlive()) {
            players[i]->update(FRAME_STEP);
        }
    }

    board->checkCollision();
    board->updateImage();
}

void ClientGameplayWindow::updatePlayersPanel() {
    ui->label->setText(players[0]->getName());

    if(players[1])
        ui->label_2->setText(players[1]->getName());

    if(players[2])
        ui->label_3->setText(players[2]->getName());

    if(players[3])
        ui->label_4->setText(players[3]->getName());
}

void ClientGameplayWindow::keyPressEvent(QKeyEvent * event) {
    if(event->key() == Qt::Key_Left)
        players[0]->setAction(Player::ACTION_LEFT);
    else if(event->key() == Qt::Key_Right)
        players[0]->setAction(Player::ACTION_RIGHT);
}

void ClientGameplayWindow::keyReleaseEvent(QKeyEvent * event) {
    if(event->key() == Qt::Key_Left && players[0]->getAction() != Player::ACTION_RIGHT)
        players[0]->setAction(Player::NO_ACTION);
    else if(event->key() == Qt::Key_Right && players[0]->getAction() != Player::ACTION_LEFT)
        players[0]->setAction(Player::NO_ACTION);
}

void ClientGameplayWindow::startGame() {
    running = true;
    connect(timer, SIGNAL(timeout()), this, SLOT(updateGameState()));
}


void ClientGameplayWindow::stopGame() {
    running = false;
    disconnect(timer, SIGNAL(timeout()), this, SLOT(updateGameState()));
}

