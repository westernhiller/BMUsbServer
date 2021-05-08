#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString params;
    if(argc > 1)
    {
        params = argv[1];
    }
    MainWindow w(params);
    w.show();

    return a.exec();
}
