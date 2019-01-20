#include "chooserolewindow.h"
#include "clientgameplaywindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ChooseRoleWindow w;
    w.show();


    return a.exec();
}
