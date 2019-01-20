#include "board.h"

Board::Board(QGraphicsView * view, Player ** players)
{
    this->view = view;
    this->view->setScene(&scene);

    for(int i = 0; i < MAX_PLAYERS; i++)
        this->players[i] = players[i];


    scene.setSceneRect(0, 0, BOARD_WIDTH, BOARD_HEIGHT);

    for(int i = 0; i < BOARD_WIDTH; i++)
        for(int j = 0; j < BOARD_HEIGHT; j++)
            collision_map[i][j] = false;

    image = new QImage(BOARD_WIDTH, BOARD_HEIGHT, QImage::Format_RGB32);
    image->fill(Qt::black);

    board_pixmap = new QGraphicsPixmapItem(QPixmap::fromImage(*image));
    scene.addItem(board_pixmap);
}

Board::Board() {
    view = 0;//nullptr;

    for(int i = 0; i < MAX_PLAYERS; i++)
        players[i] = 0;//nullptr;

    for(int i = 0; i < BOARD_WIDTH; i++)
        for(int j = 0; j < BOARD_HEIGHT; j++)
            collision_map[i][j] = false;

    image = 0;//nullptr;
    board_pixmap = 0;//nullptr;
}

void Board::clearBoard() {
    if(image) {
        image->fill(Qt::black);
        board_pixmap->setPixmap(QPixmap::fromImage(*image));
    }

    for(int i = 0; i < BOARD_WIDTH; i++)
        for(int j = 0; j < BOARD_HEIGHT; j++)
            collision_map[i][j] = false;
}

void Board::updateImage() {
    for(int i = 0; i < MAX_PLAYERS; i++) {
        if(players[i] && players[i]->isAlive()) {
            short x = players[i]->getX();
            short y = players[i]->getY();
            short size = players[i]->getSize();

            QPainter painter(image);
            painter.fillRect(x, y, size, size, QBrush(players[i]->getColor()));
        }
    }

    board_pixmap->setPixmap(QPixmap::fromImage(*image));
}


void Board::checkCollision() {
    for(int i = 0; i < MAX_PLAYERS; i++) {
        if(players[i] && players[i]->isAlive()) {
            bool players_collision = false;

            for(int j = 0; j < MAX_PLAYERS; j++) {
                if(j == i || !players[j])
                    continue;

                players_collision = checkPlayersCollision(players[i], players[j]);
            }

            if((players[i]->getIntMove()[0] == 0 && players[i]->getIntMove()[1] == 0) || players_collision)
                continue;

            int x = players[i]->getX();
            int y = players[i]->getY();
            int size = players[i]->getSize();
            int x2 = std::min(x + size, BOARD_WIDTH);
            int y2 = std::min(y + size, BOARD_HEIGHT);

            if(x < 0 || x2 > BOARD_WIDTH || y < 0 || y2 > BOARD_HEIGHT) {
                players[i]->setAlive(false);
                float pos[2];

                if(x < 0) {
                    pos[0] = 0;
                    pos[1] = players[i]->getPosition()[1];
                    players[i]->setPosition(pos);
                } else if(y < 0) {
                    pos[0] = players[i]->getPosition()[0];
                    pos[1] = 0;
                    players[i]->setPosition(pos);
                }

                continue;
            }

            for(int k = x; k < x2; k++) {
                for(int j = y; j < y2; j++) {
                    if((k >= x - players[i]->getIntMove()[0] && k < x2 - players[i]->getIntMove()[0])
                        && (j >= y - players[i]->getIntMove()[1] && j < y2 - players[i]->getIntMove()[1]))
                            continue;
                    else if(collision_map[k][j] == false) {
                        collision_map[k][j] = true;
                    } else {
                        players[i]->setAlive(false);
                        break;
                    }

                }

                if(!players[i]->isAlive())
                    break;
            }
        }
    }
}


bool Board::checkPlayersCollision(Player * p1, Player * p2) {
    if(!p1 || !p2)
        return false;

    if(p1->getX() > p2->getX() + p2->getSize() || p1->getX() + p1->getSize() < p2->getX()
       || p1->getY() > p2->getY() + p2->getSize() || p1->getY() + p1->getSize() < p2->getY())
        return false;

    p1->setAlive(false);
    p2->setAlive(false);

    return true;
}
