#ifndef CATITEMEDIT_H
#define CATITEMEDIT_H

#include <QFrame>

#include "ui_catitemframe.h"
#include "dialogtpl.h"

class QSqlQuery;

namespace STORE {
namespace Catalogue {
namespace Item {

/*************************************************************/

class Data : public QObject
{

    Q_OBJECT

public:
    Data(QObject *parent = 0) : QObject(parent), isLocal(true), pParentItem(NULL), Deleted(false) {}
    Data(QObject *parent, QSqlQuery &qry);
    // pictogramma
    QVariant Id;
    QString Code;
    QString Title;
    QDateTime From;
    QDateTime To;
    bool isLocal;
    QString Comment;
    Data *pParentItem;
    bool Deleted;
    //id

public:
    bool isActive() const;
};

/*************************************************************/

class Frame : public QFrame
{
    Q_OBJECT

public:
    Frame(QWidget *parent = 0);
    virtual ~Frame();

private:
    Ui::CatItemFrame ui;
    Data *Block;

public:
    void setDataBlock(Data *D) {Block = D; load();}

public slots:
    void is_good(bool *pOK);   
    void load();
    bool save();

signals:
    void error_message(const QString &);
};

/*************************************************************/

class Dialog : public CommonDialog
{
    Q_OBJECT

private:
    Frame *pFrame;

public:
    Dialog(QWidget *parent = 0);
    virtual ~Dialog();

public:
    void setDataBlock(Data *D) {pFrame->setDataBlock(D); }
};

/*************************************************************/

} // namespace Item

} // namespace Catalogue

} // namespace STORE

#endif // CATITEMEDIT_H

