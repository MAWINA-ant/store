#include "catalogue.h"

#include <QMenu>
#include <QtDebug>
#include <QtSql>

#include "posaction.h"

namespace STORE {
namespace Catalogue {

/**********************************************************************/

Model::Model(QObject *parent)
    : QAbstractTableModel (parent){

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
                "FROM catalogue;\n"
                );
    if (qry.exec())
    {
        while (qry.next())
        {
            Item::Data *D = new Item::Data(this, qry);
            Cat.append(D);
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
        return Cat.size(); // TODO заменить на првильное кол-во
    }
    else
    {
        return 0;
    }
}

int Model::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
    {
        return 6; // TODO заменить на првильное кол-во
    }
    else
    {
        return 0;
    }
}

QVariant Model::dataDisplay(const QModelIndex &I) const
{
    Item::Data *D = Cat.at(I.row());
    switch (I.column()){
    case 0: return D->Code;
    case 1: return D->Title;
    case 2: return D->From.isValid() ? D->From.toString("dd.MM.yyyy") : "";
    case 3: return D->To.isValid() ? D->To.toString("dd.MM.yyyy") : "";
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
    Item::Data *D = dataBlock(I);
    if (!D) return QVariant();
    QColor result = D->isLocal ? QColor("blue") : QColor("black");
    !D->isActive() ? result.setAlphaF(0.4) : result.setAlphaF(1.0);
    return result;
}

QVariant Model::dataFont(const QModelIndex &I) const
{
    Item::Data *D = dataBlock(I);
    if (!D) return QVariant();
    QFont F ;
    if (D->Deleted)
        F.setStrikeOut(true);
    return F;
}

QVariant Model::dataToolTip(const QModelIndex &I) const
{
    Item::Data *D = dataBlock(I);
    if (!D) return QVariant();
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
    //case Qt::UserRole : return QVariant(dataBlock(index));
    case Qt::UserRole+1 : {
        Item::Data *D = dataBlock(index);
        if (!D) return false;
        return D->Deleted;
    }
    default : return QVariant();
    }
}

Item::Data* Model::dataBlock(const QModelIndex &I) const
{
    int R = I.row();
    if (R < 0 || R >= Cat.size())
        return 0;
    else
        return Cat.at(R);
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QAbstractTableModel::headerData(section, orientation, role);

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
    Item::Data *D = dataBlock(I);
    if (!D) return;
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
    // TODO uverennost' v udalenii
    Item::Data *D = dataBlock(I);
    if (!D) return;
    beginResetModel();
    if (D->Id.isNull() || !D->Id.isValid()) {
        Cat.removeAt(I.row());
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
