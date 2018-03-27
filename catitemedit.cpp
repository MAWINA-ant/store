#include "catitemedit.h"

namespace STORE {
namespace Catalogue {
namespace Item {

/*************************************************************/

Frame::Frame(QWidget *parent)
    :QFrame(parent)
{
    ui.setupUi(this);
    Block = NULL;
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

/*************************************************************/

} // namespace Item

} // namespace Catalogue

} // namespace STORE
