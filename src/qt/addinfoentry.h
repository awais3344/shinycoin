#ifndef ADDINFOENTRY_H
#define ADDINFOENTRY_H

#include "txinfo.h"

#include <QFrame>

namespace Ui {
    class AddInfoEntry;
}
class WalletModel;

/** A single entry in the dialog for sending bitcoins. */
class AddInfoEntry : public QFrame
{
    Q_OBJECT

public:
    explicit AddInfoEntry(QWidget *parent = 0);
    ~AddInfoEntry();

    void setModel(WalletModel *model);
    bool validate();
    CTxInfo getValue();

    /** Return whether the entry is still empty and unedited */
    bool isClear();

    void setValue(const CTxInfo &value);

    /** Set up the tab chain manually, as Qt messes up the tab chain by default in some cases (issue http://bugreports.qt.nokia.com/browse/QTBUG-10907).
     */
    QWidget *setupTabChain(QWidget *prev);

    void setFocus();

public slots:
    void setRemoveEnabled(bool enabled);
    void keyTypeChanged(int index);
    void clear();

signals:
    void removeEntry(AddInfoEntry *entry);

private slots:
    void on_deleteButton_clicked();

private:
    Ui::AddInfoEntry *ui;
    WalletModel *model;
    
    TxInfoType currentKeyType() const;
};

#endif // ADDINFOENTRY_H
