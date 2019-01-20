#ifndef PLAYER_H
#define PLAYER_H

#include <QColor>
#include <QDebug>
#include <string>

struct PlayerData {
    QColor color;
    float position[2];
    short world_angle;
    char nickname[32];

    PlayerData(QColor c, float * pos, short world_angle, QString nick) {
        this->color = c;
        this->position[0] = pos[0];
        this->position[1] = pos[1];
        this->world_angle = world_angle;
        for(int i = 0; i < 32; i++) {
            if(i < nick.size())
                this->nickname[i] = nick.toStdString().c_str()[i];
            else
                this->nickname[i] = 0;
        }
    }
};

class Player
{
public:
    Player();
    Player(PlayerData & data);
    ~Player();

    void clearStats();
    void update(float delta);
    void spawn();

    enum Action {
        NO_ACTION = 0, ACTION_RIGHT = 1, ACTION_LEFT = -1
    };

    void setAction(Action action) { this->action = action; }
    Action getAction() { return this->action; }

    short getAngle() { return this->world_angle; }

    float * getPosition() { return this->position; }
    void setPosition(float * pos) { this->position[0] = pos[0]; this->position[1] = pos[1]; }

    short getSize() { return this->size; }

    void setAlive(bool alive) { this->alive = alive; }
    bool isAlive() { return this->alive; }

    int * getIntMove() { return int_move; }

    float * getAccMove() { return acc_move; }

    short getX() { return x; }
    void setX(short x) { this->x = x; }

    short getY() { return y; }
    void setY(short y) { this->y = y; }

    void setScore(short score) { this->score = score; }
    short getScore() { return score; }

    QString getName() { return nickname; }
    void setName(QString nickname) { this->nickname = nickname; }

    void setReady(bool is_ready) { this->is_ready = is_ready; }
    bool getReady() { return is_ready; }

    QColor getColor() { return color; }

private:
    short size;
    short x;
    short y;
    short score;
    float position[2];
    int int_move[2];
    float acc_move[2];
    short world_angle;
    short speed;
    QString nickname;
    Action action;
    bool alive;
    QColor color;
    bool is_ready;

    static const int DEFAULT_SPEED = 100;
    static const int DEFAULT_ROTATE_SPEED = 220;
};

#endif // PLAYER_H
