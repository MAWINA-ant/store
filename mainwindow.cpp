#include "mainwindow.h"
#include "catalogue.h"

namespace STORE {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    Catalogue::TableView *W = new Catalogue::TableView(this);
    setCentralWidget(W);
}

MainWindow::~MainWindow()
{

}

} // namespace STORE
