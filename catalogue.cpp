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
    case 2: return D->From;
    case 3: return D->To;
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

QVariant Model::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole : return dataDisplay(index);
    case Qt::TextAlignmentRole : return dataTextAlignment(index);
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
        QHeaderView *H = verticalHeader();
        H->setSectionResizeMode(QHeaderView::ResizeToContents);
    }
    {
        QHeaderView *H = horizontalHeader();
        H->setSectionResizeMode(QHeaderView::ResizeToContents);
        H->setSectionResizeMode(1, QHeaderView::Stretch);
    }
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
        M.exec(mapToGlobal(p));
    }
}

/**********************************************************************/

} // namespace Catalogue

} // namespace STORE
