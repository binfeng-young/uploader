//#include "mainwindow.h"
#include "uploaderwidget.h"
#include <QApplication>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //MainWindow w;
    UploaderWidget w;
    w.show();

    return a.exec();
}
