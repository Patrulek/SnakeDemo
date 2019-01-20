#ifndef CHOOSEROLEWINDOW_H
#define CHOOSEROLEWINDOW_H

#include <QMainWindow>

namespace Ui {
    class ChooseRoleWindow;
}

class ChooseRoleWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChooseRoleWindow(QWidget *parent = 0);
    ~ChooseRoleWindow();

private slots:
    void startAsServer();
    void startAsClient();

private:
    Ui::ChooseRoleWindow *ui;
};

#endif // CHOOSEROLEWINDOW_H
