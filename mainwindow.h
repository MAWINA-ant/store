#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace STORE {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();
};

} // namespace STORE

#endif // MAINWINDOW_H
