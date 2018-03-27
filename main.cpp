#include "mainwindow.h"
#include "application.h"


int main(int argc, char *argv[])
{
    STORE::Application a(argc, argv);
    STORE::MainWindow w;
    w.showMaximized();

    return a.exec();
}
