#ifndef POSACTION_H
#define POSACTION_H

#include <QAction>
#include <QModelIndex>

namespace STORE {

class PosAction : public QAction
{
    Q_OBJECT
public:
    QModelIndex I;
    QWidget *pWidget;

    PosAction(QObject *parent = 0);
    virtual ~PosAction() {}

private slots:
    void was_triggered();

signals:
    void editItem(const QModelIndex &I, QWidget *parent);
};

} // namespace STORE

#endif // POSACTION_H
