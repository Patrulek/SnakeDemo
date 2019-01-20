#ifndef CLIENTGAMEPLAYWINDOW_H
#define CLIENTGAMEPLAYWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QKeyEvent>
#include <QImage>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QTcpSocket>

#include "player.h"
#include "board.h"

#define FRAME_STEP 0.016

namespace Ui {
class ClientGameplayWindow;
}

class ClientGameplayWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClientGameplayWindow(QTcpSocket * tcpSocket, quint16 blockSize, QWidget *parent = 0);
    ~ClientGameplayWindow();

    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);

private slots:
    void startGame();
    void stopGame();
    void updateGameState();

private:
    void updateScore() {}
    void updatePlayersPanel();

    Ui::ClientGameplayWindow *ui;
    QTimer * timer;
    Player * players[MAX_PLAYERS];
    bool running;

    Board * board;

    QTcpSocket * tcpSocket;
    quint16 blockSize;
};

#endif // CLIENTGAMEPLAYWINDOW_H
