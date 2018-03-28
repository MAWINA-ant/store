#include "catalogue.h"

#include <QMenu>
#include <QtDebug>
#include <QtSql>

#include "posaction.h"

#define DATA_PTR(D, I, VALUE)                               \
    Item::Data *D = (Item::Data*)(I.internalPointer()) ;    \
    if (!D) return VALUE

#define DATE_STR(DATE) (DATE.isValid() ? DATE.toString("dd.MM.yyyy") : "")

namespace STORE {
namespace Catalogue {

/**********************************************************************/

Model::Model(QObject *parent)
    : QAbstractItemModel (parent){

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
            int parentId = qry.value("rid_parent").toInt();
            Item::Data *P = Cat.findPointer(parentId);
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
    if (!parent.isValid())
    {
        return Cat.size();
    }
    else
    {
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
    case 0: return D->Code;
    case 1: return D->Title;
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
    Result |= I.column() == 1 ? Qt::AlignLeft : Qt::AlignHCenter;
    return Result;
}

QVariant Model::dataForeground(const QModelIndex &I) const
{
    DATA_PTR(D, I, QVariant());
    QColor result = D->isLocal ? QColor("blue") : QColor("black");
    !D->isActive() ? result.setAlphaF(0.4) : result.setAlphaF(1.0);
    return result;
}

QVariant Model::dataFont(const QModelIndex &I) const
{
    DATA_PTR(D, I, QVariant());
    QFont F ;
    if (D->Deleted)
        F.setStrikeOut(true);
    return F;
}

QVariant Model::dataToolTip(const QModelIndex &I) const
{
    DATA_PTR(D, I, QVariant());
    switch (I.column()) {
    case 2: {
        if (!D->To.isValid()) return QVariant();
        return tr("Valid to: %1").arg(D->To.toString("dd.MM.yyyy"));
    }
    default: return QVariant();
    }
}

QVariant Model::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole : return dataDisplay(index);
    case Qt::TextAlignmentRole : return dataTextAlignment(index);
    case Qt::ForegroundRole : return dataForeground(index);
    case Qt::FontRole : return dataFont(index);
    case Qt::ToolTipRole : return dataToolTip(index);
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
        createIndex(row, column, (void*)D);
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
    if (P) {
        return QModelIndex();
    } else {
        int Row = -1;
        Item::Data *GP = P.pParentItem;
        if (GP) {
            for (int i = 0; i < GP->Children.size(); i++) {

            }
        }
        return createIndex(?, 0, P);
    }
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QAbstractItemModel::headerData(section, orientation, role);

    switch (role) {
    case Qt::DisplayRole :
        switch (section) {
        case 0: return tr("Code");
        case 1: return tr("Title");
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
    if (parentI.isValid()) {
        // TODO sdelat dobavlenie novogo el
        qWarning() << "Ca ";
        return;
    }
    Item::Data *D = new Item::Data(this);
    if (!D) {
        qWarning() << "cant create";
        return;
    }
    Item::Dialog Dia(parent);
    Dia.setDataBlock(D);
    if(Dia.exec() == QDialog::Accepted){
        beginResetModel();
        Cat.append(D);
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

/**********************************************************************/

TableView::TableView(QWidget *parent)
    : QTableView(parent){

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(contextMenuRequested(QPoint)));

    Model *M = new Model(this);
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
        QHeaderView *H = verticalHeader();
        H->setSectionResizeMode(QHeaderView::ResizeToContents);
    }
    {
        QHeaderView *H = horizontalHeader();
        H->setSectionResizeMode(QHeaderView::ResizeToContents);
        H->setSectionResizeMode(1, QHeaderView::Stretch);
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
    }
    actNewItem->I = QModelIndex();
    actNewItem->pWidget = this;
    M.addAction(actNewItem);
    M.exec(mapToGlobal(p));
}

/**********************************************************************/

} // namespace Catalogue

} // namespace STORE
