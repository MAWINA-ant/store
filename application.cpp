#include "application.h"

#include <QtSql>

namespace STORE {

Application::Application(int argc, char * argv[])
    : QApplication(argc, argv)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setPort(5433);
    db.setDatabaseName("store");
    db.setUserName("admin");
    db.setPassword("1");
    db.open();
}

Application::~Application()
{

}

} // namespace STORE



