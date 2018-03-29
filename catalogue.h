#ifndef CATALOGUE_H
#define CATALOGUE_H

#include <QAbstractItemModel>
#include <QTableView>

#include "catitemedit.h"

class QAction;

namespace STORE {  
class PosAction;
namespace Catalogue {

/*******************************************************************************/

class Model : public QAbstractItemModel {

    Q_OBJECT

private:
    mutable int LastTempId;
    Item::List Cat;

protected:
    virtual QVariant dataDisplay(const QModelIndex &I) const;
    virtual QVariant dataTextAlignment (const QModelIndex &I) const;
    virtual QVariant dataForeground (const QModelIndex &I) const;
    virtual QVariant dataBackground (const QModelIndex &I) const;
    virtual QVariant dataFont (const QModelIndex &I) const;
    virtual QVariant dataToolTip (const QModelIndex &I) const;
    bool delete_all();
    bool save_all();
    bool insert_all();

private:
    bool delete_all_from_db(Item::Data *D = 0);
    bool delete_all_from_model(Item::Data *D = 0);
    bool save_all_to_db(Item::Data *D = 0);
    bool drop_change_mark(Item::Data *D = 0);
    bool insert_all_to_db(Item::Data *D = 0);
    bool adjust_id_for_new(Item::Data *D = 0);


    int tempId() const {return ++LastTempId;}

public:
    Model(QObject *parent = 0);
    virtual ~Model();

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;


    QVariant headerData(int section,
                        Qt::Orientation orientation, int role) const;
public slots:
    void editItem(const QModelIndex &I      , QWidget *parent = 0);
    void newItem(const QModelIndex &parentI , QWidget *parent = 0);
    void delItem(const QModelIndex &I       , QWidget *parent = 0);
    void save(void);

public:

};

/*******************************************************************************/

class TableView : public QTableView {
    Q_OBJECT

private:
    PosAction *actEditItem;
    PosAction *actNewItem;
    PosAction *actDelItem;
    PosAction *actRootItem;
    QAction *actParentRootItem;
    QAction *actSave;

public:
    TableView(QWidget *parent = 0);
    virtual ~TableView();

private slots:
    void contextMenuRequested(const QPoint &p);
    void ShowChildren(const QModelIndex &I, QWidget*);
    void ShowParent  ();
};

/*******************************************************************************/


} // namespace Catalogue

} // namespace STORE

#endif // CATALOGUE_H

