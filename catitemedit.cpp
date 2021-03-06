#include "catitemedit.h"

#include <QtSql>

namespace STORE {
namespace Catalogue {
namespace Item {

/*************************************************************/

Frame::Frame(QWidget *parent)
    :QFrame(parent)
{
    ui.setupUi(this);
    Block = NULL;
    ui.edtFrom->setDateTime(QDateTime::currentDateTime());
}

Frame::~Frame()
{

}

void Frame::load()
{
    if (!Block)
        return;
    ui.edtCode->setText(Block->Code);
    ui.edtTitle->setText(Block->Title);
    ui.cbxLocal->setChecked(Block->isLocal);
    ui.cbxFromEnabled->setChecked(Block->From.isValid());
    ui.edtFrom->setDateTime(Block->From);
    ui.cbxToEnabled->setChecked(Block->To.isValid());
    ui.edtTo->setDateTime(Block->To);
    ui.edtComment->setText(Block->Comment);

}

bool Frame::save()
{
    if (!Block) return false;

    Block->Code = ui.edtCode->text().simplified();
    Block->Title = ui.edtTitle->text().simplified();
    if (ui.cbxFromEnabled->isChecked())
        Block->From = ui.edtFrom->dateTime();
    else
        Block->From = QDateTime();
    if (ui.cbxToEnabled->isChecked())
        Block->To = ui.edtTo->dateTime();
    else
        Block->To = QDateTime();
    Block->isLocal = ui.cbxLocal->isChecked();
    Block->Comment = ui.edtComment->toPlainText().trimmed();
    Block->Changed = true;
    return true;
}

/** TODO : Тут сделать норм проверку*/

void Frame::is_good(bool *pOK)
{
    *pOK = true;
}

/*************************************************************/

Dialog::Dialog(QWidget *parent)
    : CommonDialog(parent)
{
    Frame *F = pFrame = new Frame(this);
    setCentralFrame(F);
}

Dialog::~Dialog()
{

}

Data::Data(QObject *parent, QSqlQuery &qry)
    : QObject(parent)
{
    Id = qry.value("iid");
    Code = qry.value("code").toString();
    Title = qry.value("title").toString();
    From = qry.value("valid_from").toDateTime();
    To = qry.value("valid_to").toDateTime();
    isLocal = qry.value("isLocal").toBool();
    Comment = qry.value("acomment").toString();
    pParentItem = NULL;
    Deleted = false;
    Changed = false;
}

bool Data::isActive() const
{
    if (From.isValid())
        if (From > QDateTime::currentDateTime())
            return false;
    if (To.isValid())
        if (To < QDateTime::currentDateTime())
            return false;
    return true;
}

bool Data::isNew() const
{
    if (!Id.isValid()) return true;
    if (Id.isNull()) return true;
    return false;
}

bool Data::isSameAs(Data *D) const
{
    if (isNew()) {
        if (!D->isNew()) return false;
        return property("temp_id") == D->property("temp_id");
    } else {
        if (D->isNew()) return false;
        return D->Id == Id;
    }
}

Data *List::findPointer(int Id) const
{
    Data *D;
    foreach (D, *this) {
        bool OK;
        int cid = D->Id.toInt(&OK);
        if (OK && cid == Id)
            return D;
        Data *R = D->Children.findPointer(Id);
        if (R) return R;
    }
    return 0;
}

/*************************************************************/

} // namespace Item

} // namespace Catalogue

} // namespace STORE
