#include "dialogtpl.h"

#include <QMessageBox>
#include <QLayout>

namespace STORE {

/*************************************************************/

ButtonsDlg::ButtonsDlg(QWidget *parent)
    : QFrame(parent)
{
    ui.setupUi(this);
    connect(ui.ptnOk, SIGNAL(clicked(bool)), this, SIGNAL(accepted()));
    connect(ui.btnCancel, SIGNAL(clicked(bool)), this, SIGNAL(rejected()));
}

ButtonsDlg::~ButtonsDlg()
{

}

void ButtonsDlg::error_message(const QString & msg)
{
    ui.lblErrorMsg->setText(msg);
}

/*************************************************************/

CommonDialog::CommonDialog (QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *L = new QVBoxLayout(this);
    {
        ButtonsDlg *B = pButtons = new ButtonsDlg(this);
        connect(B, SIGNAL(rejected()), this, SLOT(reject()));
        connect(B, SIGNAL(accepted()), this, SLOT(accept_pressed()));
        L->addWidget(B);
    }
}

CommonDialog::~CommonDialog()
{

}

void CommonDialog::setCentralFrame(QFrame *central)
{
    QVBoxLayout *L = qobject_cast<QVBoxLayout*>(layout());
    central->setParent(this);
    pFrame = central;
    L->insertWidget(0, pFrame);

    connect (pFrame, SIGNAL(error_message(const QString &)),
             pButtons, SLOT(error_message(const QString)));
    connect (this, SIGNAL(check_data(bool*)), pFrame, SLOT(is_good(bool*)));
    connect(this, SIGNAL(save()), pFrame, SLOT(save()));
}

void CommonDialog::accept_pressed()
{
    bool OK = true;
    emit check_data(&OK);
    if (OK) {
        emit save();
        accept();
    }

    //QMessageBox::information(this, 'Attention', 'Accept data slot');
}

/*************************************************************/

} // namespace STORE
