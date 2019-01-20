#include "player.h"

#include <QBrush>
#include <cmath>

#include <QDebug>

Player::Player()
{
    color.setRgb(qrand() % 206 + 50, qrand() % 206 + 50, qrand() % 206 + 50);
    size = 7;
    score = 0;
    nickname = "playerx\nscore: 0";
    is_ready = false;
    spawn();
}

Player::Player(PlayerData & data)
{
    color = data.color;
    position[0] = data.position[0];
    position[1] = data.position[1];
    world_angle = data.world_angle;
    nickname = QString((const char *)data.nickname);

    x = (int)position[0];
    y = (int)position[1];
    size = 7;
    score = 0;
    acc_move[0] = acc_move[1] = 0;
    action = NO_ACTION;
    speed = DEFAULT_SPEED;
    alive = true;
    is_ready = false;
}

Player::~Player()
{

}

void Player::clearStats() {
    score = 0;
    is_ready = false;
    alive = true;
    action = NO_ACTION;
}

void Player::update(float delta)
{
    world_angle = (int)(world_angle + action * DEFAULT_ROTATE_SPEED * delta) % 360;
    float rad = (float)world_angle / 180.0 * 3.1415;

    float moveX = speed * sin(rad) * delta;
    float moveY = speed * (-cos(rad)) * delta;

    acc_move[0] += moveX;
    acc_move[1] += moveY;

    position[0] += moveX;
    position[1] += moveY;

    int_move[0] = acc_move[0] < 0 ? std::ceil(acc_move[0]) : std::floor(acc_move[0]);
    acc_move[0] -= (float)int_move[0];

    int_move[1] = acc_move[1] < 0 ? std::ceil(acc_move[1]) : std::floor(acc_move[1]);
    acc_move[1] -= (float)int_move[1];

    x += int_move[0];
    y += int_move[1];
}


void Player::spawn()
{
    position[0] = qrand() % 500 + 100;
    position[1] = qrand() % 400 + 100;
    world_angle = qrand() % 360;
    x = (int)position[0];
    y = (int)position[1];
    acc_move[0] = acc_move[1] = 0;
    action = NO_ACTION;
    speed = DEFAULT_SPEED;
    alive = true;
}

