#include "catalogue.h"

#include <QMenu>
#include <QtDebug>
#include <QtSql>
#include <QMessageBox>

#include "posaction.h"

#define DATA_PTR(D, I, VALUE)                               \
    Item::Data *D = (Item::Data*)(I.internalPointer()) ;    \
    if (!D) return VALUE

#define DATE_STR(DATE) (DATE.isValid() ? DATE.toString("dd.MM.yyyy") : "")

namespace STORE {
namespace Catalogue {

/**********************************************************************/

Model::Model(QObject *parent)
    : QAbstractItemModel (parent) , LastTempId(1) {

    QSqlQuery qry;
    qry.setForwardOnly(true);
    qry.prepare(
                "SELECT         \n"
                "iid ,          \n"
                "code,          \n"
                "title,         \n"
                "valid_from,    \n"
                "valid_to,      \n"
                "isLocal,       \n"
                "acomment,      \n"
                "rid_parent,    \n"
                "alevel         \n"
                "FROM catalogue \n"
                "ORDER by alevel ; \n"
                );
    if (qry.exec())
    {
        while (qry.next())
        {
            bool OK = false ;
            QVariant PI = qry.value("rid_parent");
            int parentId = PI.toInt(&OK);
            Item::Data *P = OK && !PI.isNull() ? Cat.findPointer(parentId) : 0;
            if (P){
                Item::Data *D = new Item::Data(P, qry);
                P->Children.append(D);
                D->pParentItem = P;
            } else {
                Item::Data *D = new Item::Data(this, qry);
                Cat.append(D);
                D->pParentItem = 0;
            }
        }
    }
    else
    {
        QSqlError Err = qry.lastError();
        qCritical() << Err.nativeErrorCode();
        qCritical() << Err.databaseText();
        qCritical() << Err.driverText();
    }
}

Model::~Model() {

}

int Model::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return Cat.size();
    } else {
        DATA_PTR(P, parent, 0);
        return P->Children.size();
    }
}

int Model::columnCount(const QModelIndex &parent) const
{
    return 6;
}

QVariant Model::dataDisplay(const QModelIndex &I) const
{
    DATA_PTR(D, I, QVariant());
    switch (I.column()){
    case 1: return D->Code;
    case 0: return D->Title;
    case 2: return DATE_STR(D->From);
    case 3: return DATE_STR(D->To);
    case 4: return D->isLocal ? tr("LOCAL") : QString();
    case 5: return D->Comment.isEmpty() ? QString() : tr("CMT");
    default: return QVariant();
    }
}

QVariant Model::dataTextAlignment(const QModelIndex &I) const
{
    int Result = Qt::AlignVCenter;
    Result |= I.column() == 0 ? Qt::AlignLeft : Qt::AlignHCenter;
    return Result;
}

QVariant Model::dataForeground(const QModelIndex &I) const
{
    DATA_PTR(D, I, QVariant());
    QColor result = D->isLocal ? QColor("blue") : QColor("black");
    !D->isActive() ? result.setAlphaF(0.4) : result.setAlphaF(1.0);

    return result;
}

QVariant Model::dataBackground(const QModelIndex &I) const
{
    DATA_PTR(D, I, QVariant());
    QColor result = D->isNew() ? QColor("green") : QColor("white");
    result.setAlphaF(0.1);
    return result;
}

QVariant Model::dataFont(const QModelIndex &I) const
{
    DATA_PTR(D, I, QVariant());
    QFont F ;
    if (D->Deleted)
        F.setStrikeOut(true);
    if (D->Changed)
        F.setItalic(true);
    return F;
}

QVariant Model::dataToolTip(const QModelIndex &I) const
{
    DATA_PTR(D, I, QVariant());
    //switch (I.column()) {
    //case 2: {
        if (!D->To.isValid()) return QVariant();
        return tr("Valid to: %1").arg(D->To.toString("dd.MM.yyyy"));
    //}
    //default: return QVariant();
    //}
}

bool Model::delete_all()
{
    QSqlDatabase DB = QSqlDatabase::database();
    DB.transaction();
    if (delete_all_from_db()) {
        DB.commit();
        return delete_all_from_model();
    } else {
        DB.rollback();
        return false;
    }
}

bool Model::save_all()
{
    QSqlDatabase DB = QSqlDatabase::database();
    DB.transaction();
    if (save_all_to_db()) {
        DB.commit();
        return drop_change_mark();
    } else {
        DB.rollback();
        return false;
    }
}

bool Model::insert_all()
{
    QSqlDatabase DB = QSqlDatabase::database();
    DB.transaction();
    if (insert_all_to_db()) {
        DB.commit();
        return adjust_id_for_new();
    } else {
        DB.rollback();
        return false;
    }
}

bool Model::delete_all_from_db(Item::Data *D)
{
    Item::List *Children = D ? &(D->Children) : &Cat;
    Item::Data *X;
    foreach (X, *Children) {
        if (!delete_all_from_db(X))
            return false;
    }
    if (!D) return true;
    if (!D->Deleted) return true;
    {
        QSqlQuery DEL;
        DEL.setForwardOnly(true);
        DEL.prepare("delete from catalogue where iid = :IID");
        DEL.bindValue(":IID", D->Id);
        if (DEL.exec()) return true;
        qCritical() << DEL.lastError().databaseText();
        qCritical() << DEL.lastError().driverText();
        qCritical() << DEL.lastError().nativeErrorCode();
    }
    return false;
}

bool Model::delete_all_from_model(Item::Data *D)
{
    Item::List *Children = D ? &(D->Children) : &Cat;
    bool Result = true;
    beginResetModel();
    for (int k = Children->size() - 1; k >= 0 ; k--) {
        if (Children->at(k)->Deleted) {
            Item::Data *X = Children->takeAt(k);
            delete X;
        } else {
            if (!delete_all_from_model(Children->at(k))){
                Result = false;
                break;
            }
        }
    }
    endResetModel();
    return Result;
}

bool Model::save_all_to_db(Item::Data *D)
{
    Item::List *Children = D ? &(D->Children) : &Cat;
    Item::Data *X;
    foreach (X, *Children) {
        if (!save_all_to_db(X))
            return false;
    }
    if (!D) return true;
    if (!D->Changed) return true;
    {
        QSqlQuery UPD;
        UPD.setForwardOnly(true);
        UPD.prepare("update catalogue set       \n"
                    "    code = :CODE,          \n"
                    "    title      = :TITLE,   \n"
                    "    valid_from = :FROM,    \n"
                    "    valid_to   = :TO,      \n"
                    "    islocal    = :LOCAL,   \n"
                    "    acomment   = :COMMENT  \n"
                    "    where iid = :IID"
                    );

        UPD.bindValue(":CODE", D->Code);
        UPD.bindValue(":TITLE", D->Title);
        UPD.bindValue(":FROM", D->From);
        UPD.bindValue(":TO", D->To);
        UPD.bindValue(":LOCAL", D->isLocal);
        UPD.bindValue(":COMMENT", D->Comment);
        UPD.bindValue(":IID", D->Id);
        if (UPD.exec()) return true;
        qCritical() << UPD.lastError().databaseText();
        qCritical() << UPD.lastError().driverText();
        qCritical() << UPD.lastError().nativeErrorCode();
    }
    return false;
}

bool Model::drop_change_mark(Item::Data *D)
{
    Item::List *Children = D ? &(D->Children) : &Cat;
    Item::Data *X;
    foreach (X, *Children) {
        X->Changed = false;
        drop_change_mark(X);
    }
    return true;
}

bool Model::insert_all_to_db(Item::Data *D)
{
    bool must_be_saved = D ? D->isNew() : false;
    if (must_be_saved) {
        QSqlQuery INS;
        INS.setForwardOnly(true);
        INS.prepare(
                    "insert into catalogue (            \n"
                    "       code ,       title ,        \n"
                    "       valid_from , valid_to ,     \n"
                    "       islocal ,    acomment ,     \n"
                    "       rid_parent                  \n"
                    ") values (                         \n"
                    "       :CODE ,      :TITLE ,       \n"
                    "       :FROM ,      :TO ,          \n"
                    "       :LOCAL ,     :COMMENT ,     \n"
                    "       :PARENT                     \n"
                    ") returning iid ,                  \n"
                    "       code ,      title ,         \n"
                    "       valid_from , valid_to ,     \n"
                    "       islocal ,    acomment ,     \n"
                    "       rid_parent , alevel         \n"
                    );
        INS.bindValue(":CODE", D->Code);
        INS.bindValue(":TITLE", D->Title);
        INS.bindValue(":FROM", D->From);
        INS.bindValue(":TO", D->To);
        INS.bindValue(":LOCAL", D->isLocal);
        INS.bindValue(":COMMENT", D->Comment);
        QVariant IdParent = QVariant();
        if (D->pParentItem) {
            if (D->pParentItem->isNew()) {
                IdParent = D->pParentItem->property("new_id");
            } else {
                IdParent = D->pParentItem->Id;
            }
        }
        INS.bindValue(":PARENT", IdParent);
        if (INS.exec()) {
            while (INS.next()) {
                D->Code = INS.value("code").toString();
                D->Title = INS.value("title").toString();
                D->From = INS.value("valid_from").toDateTime();
                D->To = INS.value("valid_to").toDateTime();
                D->isLocal = INS.value("isLocal").toBool();
                D->Comment = INS.value("acomment").toString();
                qDebug() << INS.value("rid_parent") << INS.value("alevel");
                D->setProperty("new_id", INS.value("iid"));
            }
            return true;
        } else {
            qCritical() << INS.lastError().databaseText();
            qCritical() << INS.lastError().driverText();
            qCritical() << INS.lastError().nativeErrorCode();
            return false;
        }

    }
    Item::List *Children = D ? &(D->Children) : &Cat;
    Item::Data *X;
    foreach (X, *Children) {
        if (!insert_all_to_db(X))
            return false;
    }
    return true;
}

bool Model::adjust_id_for_new(Item::Data *D)
{
    bool must_be_saved = D ? D->isNew() : false;
    if (must_be_saved) {
        D->Id = D->property("new_id");
    }
    Item::List *Children = D ? &(D->Children) : &Cat;
    Item::Data *X;
    foreach (X, *Children) {
        if (adjust_id_for_new(X))
            return false;
    }
    return true;
}

QVariant Model::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole : return dataDisplay(index);
    case Qt::TextAlignmentRole : return dataTextAlignment(index);
    case Qt::ForegroundRole : return dataForeground(index);
    case Qt::FontRole : return dataFont(index);
    case Qt::ToolTipRole : return dataToolTip(index);
    case Qt::BackgroundRole : return dataBackground(index);
    case Qt::UserRole+1 : {
        DATA_PTR(D, index, false);
        return D->Deleted;
    }
    default : return QVariant();
    }
}

QModelIndex Model::index(int row, int column,
                         const QModelIndex &parent) const
{
    if (parent.isValid()) {
        DATA_PTR(D, parent, QModelIndex());
        if (row < 0 || row >= D->Children.size())
            return QModelIndex();
        return createIndex(row, column, (void*)(D->Children.at(row)));
    } else {
        if (row < 0 || row >= Cat.size())
            return QModelIndex();
        return createIndex(row, column, (void*)Cat.at(row));
    }
}

QModelIndex Model::parent(const QModelIndex &child) const
{
    DATA_PTR(D, child, QModelIndex());
    Item::Data *P = D->pParentItem;
    if (!P) {
        return QModelIndex();
    } else {
        int Row = -1;
        Item::Data *GP = P->pParentItem;
        if (GP) {
            for (int i = 0; i < GP->Children.size(); i++) {
                if (GP->Children.at(i)->isSameAs(P)) {
                    Row = i;
                    break;
                }
            }
        } else {
            for (int i = 0; i < Cat.size(); i++) {
                if (Cat.at(i)->isSameAs(P)) {
                    Row = i;
                    break;
                }
            }
        }
        if (Row < 0) {
            qWarning() << "Cannot find item";
            return QModelIndex();
        }
        return createIndex(Row, 0, P);
    }
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QAbstractItemModel::headerData(section, orientation, role);

    switch (role) {
    case Qt::DisplayRole :
        switch (section) {
        case 1: return tr("Code");
        case 0: return tr("Title");
        case 2: return tr("From");
        case 3: return tr("To");
        case 4: return tr("Local");
        }
    case Qt::TextAlignmentRole :
        return QVariant(Qt::AlignBaseline | Qt::AlignHCenter);
    case Qt::ForegroundRole :
    { // TODO Sdelat' shrift zhirnym
        QFont F;
        F.setBold(true);
        return F;
    }
    default: return QVariant();
    }

}

void Model::editItem(const QModelIndex &I, QWidget *parent)
{
    Item::Dialog Dia(parent);
    DATA_PTR(D,I,);
    Dia.setDataBlock(D);
    beginResetModel();
    Dia.exec();
    endResetModel();
}

void Model::newItem(const QModelIndex &parentI, QWidget *parent)
{
    Item::Data *P = 0;
    if (parentI.isValid()) {
        P = (Item::Data*)(parentI.internalPointer());
        if (!P) {
            qWarning() << "invalid internal pointer";
            return;
        }
    }
    Item::Data *D = new Item::Data(this);
    if (!D) {
        qWarning() << "cant create";
        return;
    }
    Item::Dialog Dia(parent);
    Dia.setDataBlock(D);
    if(Dia.exec() == QDialog::Accepted) {
        beginResetModel();
        if (P) {
            P->Children.append(D);
            D->pParentItem = P;
        } else {
            Cat.append(D);
        }
        D->setProperty("temp_id", tempId());
        endResetModel();

    }
    else
    {
        delete D;
    }
}

void Model::delItem(const QModelIndex &I, QWidget *parent)
{
    DATA_PTR(D,I,);
    beginResetModel();
    if (D->isNew()) {
        Item::Data *P = D->pParentItem;
        if (P) {
            P->Children.removeAt(I.row());
        } else {
            Cat.removeAt(I.row());
        }
        delete D;
    }
    else {
        D->Deleted = !D->Deleted;
    }
    endResetModel();
}

void Model::save( void )
{
    try {
        if (!delete_all()) throw (int)1;
        if (!save_all()) throw (int)2;
        if (!insert_all()) throw (int)3;
    } catch (int x) {
        QMessageBox::critical(0, tr("Error"), tr("Cannot save changes"));
    }
}

/**********************************************************************/

TableView::TableView(QWidget *parent, Model *xModel)
    : QTableView(parent){

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(contextMenuRequested(QPoint)));

    Model *M = xModel ? xModel : new Model(this);
    setModel(M);

    {
        PosAction *A = actEditItem = new PosAction(this);
        A->setText(tr("Edit"));
        connect(A, SIGNAL(editItem(QModelIndex,QWidget*)), M, SLOT(editItem(QModelIndex,QWidget*)));
        addAction(A);
    }

    {
        PosAction *A = actNewItem = new PosAction(this);
        A->setText(tr("Add"));
        connect(A, SIGNAL(editItem(QModelIndex,QWidget*)), M, SLOT(newItem(QModelIndex,QWidget*)));
        addAction(A);
    }

    {
        PosAction *A = actDelItem = new PosAction(this);
        A->setText(tr("Delete"));
        connect(A, SIGNAL(editItem(QModelIndex,QWidget*)), M, SLOT(delItem(QModelIndex,QWidget*)));
        addAction(A);
    }

    {
        PosAction *A = actRootItem = new PosAction(this);
        A->setText(tr("Show children"));
        connect(A, SIGNAL(editItem(QModelIndex,QWidget*)), this, SLOT(ShowChildren(QModelIndex,QWidget*)));
        addAction(A);
    }

    {
        QAction *A = actParentRootItem = new QAction(this);
        A->setText(tr("Show parent"));
        connect(A, SIGNAL(triggered(bool)), this, SLOT(ShowParent()));
        addAction(A);
    }

    {
        QAction *A = actSave = new QAction(this);
        A->setText(tr("Save"));
        connect(A, SIGNAL(triggered(bool)), M, SLOT(save()));
        addAction(A);
    }

    {
        QHeaderView *H = verticalHeader();
        H->setSectionResizeMode(QHeaderView::ResizeToContents);
    }
    {
        QHeaderView *H = horizontalHeader();
        H->setSectionResizeMode(QHeaderView::ResizeToContents);
        H->setSectionResizeMode(0, QHeaderView::Stretch);
    }
    setColumnHidden(3, true);
    setColumnHidden(4, true);
}

TableView::~TableView()
{

}

void TableView::contextMenuRequested(const QPoint &p)
{
    QMenu M(this);
    QModelIndex I = indexAt(p);
    if (I.isValid()) {
        actEditItem->I = I;
        actEditItem->pWidget = this;
        M.addAction(actEditItem);
        actDelItem->I = I;
        actDelItem->pWidget = this;
        if (I.data(Qt::UserRole+1).toBool()){
            actDelItem->setText(tr("Restore"));
        }
        else {
            actDelItem->setText(tr("Delete"));
        }
        M.addAction(actDelItem);
        actRootItem->I = I;
        actRootItem->pWidget = this;
        M.addAction(actRootItem);
    }
    actNewItem->I = rootIndex();
    actNewItem->pWidget = this;
    M.addAction(actNewItem);
    if (rootIndex().isValid())
        M.addAction(actParentRootItem);
    M.addAction(actSave);
    M.exec(mapToGlobal(p));
}

void TableView::ShowChildren(const QModelIndex &I, QWidget* )
{
    setRootIndex(I);
}

void TableView::ShowParent(void)
{
    if (rootIndex().isValid())
        setRootIndex(rootIndex().parent());
}

TreeView::TreeView(QWidget *parent, Model *xModel)
    : QTreeView(parent)
{
    Model *M = xModel ? xModel : new Model(this);
    setModel(M);
    {
        QHeaderView *H = header();
        H->setSectionResizeMode(QHeaderView::ResizeToContents);
        H->setSectionResizeMode(1, QHeaderView::Stretch);
    }
    setColumnHidden(3, true);
    setColumnHidden(4, true);
}

TreeView::~TreeView()
{

}

ColumnView::ColumnView(QWidget *parent, Model *xModel)
    : QColumnView(parent)
{
    Model *M = xModel ? xModel : new Model(this);
    setModel(M);

    QList<int> L;
    L << 150;
    for (int k = 0; k < 10; k ++) L << 200;
    setColumnWidths(L);
}

ColumnView::~ColumnView()
{

}

void ColumnView::currentChanged(const QModelIndex &current,
                                const QModelIndex &previous)
{
    QColumnView::currentChanged(current, previous);
    if (!current.isValid()) {
        emit itemSelected(QVariant());
        return;
    }
    Item::Data *D = (Item::Data*)(current.internalPointer()) ;
    if (!D) {
        emit itemSelected(QVariant());
        return;
    }
    emit itemSelected(D->Id);
}

/**********************************************************************/

} // namespace Catalogue

} // namespace STORE
