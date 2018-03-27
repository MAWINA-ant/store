#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

namespace STORE {

class Application : public QApplication
{

    Q_OBJECT

public:
    Application(int argc, char *argv[]);
    virtual ~Application();
};

} // namespace STORE

#endif // APPLICATION_H
