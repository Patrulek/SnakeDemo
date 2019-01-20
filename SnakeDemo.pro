#-------------------------------------------------
#
# Project created by QtCreator 2015-12-07T18:34:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
                                  QT += network

TARGET = SnakeDemo
TEMPLATE = app


SOURCES += main.cpp \
    serverwindow.cpp \
    chooserolewindow.cpp \
    player.cpp \
    board.cpp \
    clientwindow.cpp \
    message.cpp

HEADERS  += \
    serverwindow.h \
    chooserolewindow.h \
    player.h \
    board.h \
    clientwindow.h \
    message.h

FORMS    += \
    serverwindow.ui \
    clientjoinwindow.ui \
    clientgameplaywindow.ui \
    chooserolewindow.ui
