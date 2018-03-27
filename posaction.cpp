#include "posaction.h"

namespace STORE {

PosAction::PosAction(QObject *parent)
    : QAction(parent)
{
    connect(this, SIGNAL(triggered(bool)), this, SLOT(was_triggered()));
}


void PosAction::was_triggered()
{
    emit editItem(I, pWidget);
}

}
