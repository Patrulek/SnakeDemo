#ifndef BOARD_H
#define BOARD_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>

#include "player.h"

#define BOARD_WIDTH 700
#define BOARD_HEIGHT 600

#define MAX_PLAYERS 4

class Board
{
public:
    Board(QGraphicsView * view, Player ** players);
    Board();

    void updateImage();
    void clearBoard();
    void checkCollision();

    void setPlayer(Player * player, int index) { this->players[index] = player; }

private:
    QGraphicsScene scene;
    QGraphicsView * view;
    QGraphicsPixmapItem * board_pixmap;
    QImage * image;
    bool collision_map[BOARD_WIDTH][BOARD_HEIGHT];
    Player * players[MAX_PLAYERS];

    bool checkPlayersCollision(Player * p1, Player * p2);
};

#endif // BOARD_H
