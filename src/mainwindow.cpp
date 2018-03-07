#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "uploaderwidget.h"
#include "serialportwidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SerialPortWidget *serialPortWidget = new SerialPortWidget(this);
    ui->tabWidget->addTab(serialPortWidget, "seralport");
    UploaderWidget *uploaderWidget = new UploaderWidget(this);
    ui->tabWidget->addTab(uploaderWidget, "upload");


}

MainWindow::~MainWindow()
{
    delete ui;
}
