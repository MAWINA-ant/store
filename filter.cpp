#include "filter.h"

namespace STORE {

Filter::Filter(QWidget *parent)
    : QFrame(parent)
{
    ui.setupUi(this);
    ui.btnFilter->setDefaultAction(ui.actionApplyFilter);
    connect(ui.actionApplyFilter, SIGNAL(triggered()), this, SLOT(apply_filter_triggered()));
}

Filter::~Filter()
{

}

void Filter::apply_filter_triggered()
{
    emit apply_filter(this);
}

QVariant Filter::author() const
{
    QString T = ui.edtAuthor->text().simplified();
    return T.isEmpty() ? QVariant() : T;
}

QVariant Filter::title() const
{
    QString T = ui.edtTitle->text().simplified();
    return T.isEmpty() ? QVariant() : T;}

QVariant Filter::year() const
{
    QString T = ui.edtYear->text().simplified();
    if (T.isEmpty()) return QVariant();
    bool OK;
    int Result = T.toInt(&OK);
    return OK ? Result : QVariant();
}

} // namespace STORE


