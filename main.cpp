#include "mainwindow.h"
#include "application.h"

int main(int argc, char *argv[])
{
    STORE::Application a(argc, argv);
    STORE::MainWindow w;
    w.show();

    return a.exec();
}
