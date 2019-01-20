#include "chooserolewindow.h"
#include "ui_chooserolewindow.h"

#include "serverwindow.h"
#include "clientwindow.h"

#include <QCloseEvent>
#include <QMessageBox>

ChooseRoleWindow::ChooseRoleWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChooseRoleWindow)
{
    ui->setupUi(this);

    connect(ui->btn_server, SIGNAL(clicked(bool)), this, SLOT(startAsServer()));
    connect(ui->btn_client, SIGNAL(clicked(bool)), this, SLOT(startAsClient()));
}

ChooseRoleWindow::~ChooseRoleWindow()
{
    delete ui;
}

void ChooseRoleWindow::startAsServer()
{
    ServerWindow * w = new ServerWindow();
    w->show();
    w->setAttribute(Qt::WA_DeleteOnClose);
    this->destroy();
}

void ChooseRoleWindow::startAsClient()
{
    ClientWindow * w = new ClientWindow();
    w->show();
    w->setAttribute(Qt::WA_DeleteOnClose);
    this->destroy();
}
