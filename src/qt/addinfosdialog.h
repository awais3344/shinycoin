#ifndef ADDINFOSDIALOG_H
#define ADDINFOSDIALOG_H

#include "txinfo.h"

#include <QDialog>

namespace Ui {
    class AddInfosDialog;
}
class WalletModel;
class AddInfoEntry;
class SendCoinsRecipient;

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

class AddInfosDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AddInfosDialog(QWidget *parent = 0);
    ~AddInfosDialog();
    
    void setModel(WalletModel *model);
    
    /** Set up the tab chain manually, as Qt messes up the tab chain by default in some cases (issue http://bugreports.qt.nokia.com/browse/QTBUG-10907).
     */
    QWidget *setupTabChain(QWidget *prev);
    
    void pasteEntry(const CTxInfo &rv);
    
public slots:
    void clear();
    void reject();
    void accept();
    AddInfoEntry *addEntry();
    void updateRemoveEnabled();
    
private:
    Ui::AddInfosDialog *ui;
    WalletModel *model;
    bool fNewRecipientAllowed;
    
private slots:
    void on_submitButton_clicked();
    void on_addressBookButton_clicked();
    
    void removeEntry(AddInfoEntry* entry);
};

#endif // ADDINFOSDIALOG_H
