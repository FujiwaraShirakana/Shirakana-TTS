#include "svcmainwindow.h"
#include "ui_svcmainwindow.h"

SVCMainWindow::SVCMainWindow(const std::function<void(QDialog*)>& _callback, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SVCMainWindow),
    exit_callback(_callback)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
}

SVCMainWindow::~SVCMainWindow()
{
    delete ui;
}
