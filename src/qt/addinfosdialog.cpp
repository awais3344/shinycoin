#include "addinfosdialog.h"
#include "ui_addinfosdialog.h"
#include "walletmodel.h"
#include "bitcoinunits.h"
#include "addressbookpage.h"
#include "optionsmodel.h"
#include "addinfoentry.h"
#include "guiutil.h"
#include "askpassphrasedialog.h"

#include <QMessageBox>
#include <QLocale>
#include <QTextDocument>
#include <QScrollBar>
#include <QTimer>

AddInfosDialog::AddInfosDialog(QWidget *parent) : QDialog(parent), ui(new Ui::AddInfosDialog), model(0)
{
    ui->setupUi(this);
    
#ifdef Q_WS_MAC // Icons on push buttons are very uncommon on Mac
    ui->addButton->setIcon(QIcon());
    ui->clearButton->setIcon(QIcon());
    ui->submitButton->setIcon(QIcon());
#endif
    
    addEntry();
    
    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addEntry()));
    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clear()));
    
    fNewRecipientAllowed = true;
}

void AddInfosDialog::setModel(WalletModel *model)
{
    this->model = model;
    
    for(int i = 0; i < ui->entries->count(); ++i)
    {
        AddInfoEntry *entry = qobject_cast<AddInfoEntry*>(ui->entries->itemAt(i)->widget());
        if(entry)
        {
            entry->setModel(model);
        }
    }
}

AddInfosDialog::~AddInfosDialog()
{
    delete ui;
}

void AddInfosDialog::on_submitButton_clicked()
{
    QList<CTxInfo> infos;
    bool valid = true;
    
    if(!model)
        return;
    
    if (!model->ownValidAddress(ui->addressEntry->text())) {
        ui->addressEntry->setValid(false);
        valid = false;
    }
    
    for(int i = 0; i < ui->entries->count(); ++i)
    {
        AddInfoEntry *entry = qobject_cast<AddInfoEntry*>(ui->entries->itemAt(i)->widget());
        if(entry)
        {
            if(entry->validate())
            {
                infos.append(entry->getValue());
            }
            else
            {
                valid = false;
            }
        }
    }
    
    if(!valid || infos.isEmpty())
    {
        return;
    }
    
    // Format confirmation message
    QStringList formatted;
    int warningLevel = 0;
    foreach(const CTxInfo &info, infos)
    {
        formatted.append(tr("&nbsp;&nbsp;&nbsp;&nbsp;<b>%1</b>: %2").arg(info.key.ToString().c_str(), Qt::escape(info.value.c_str())));
        if (info.key.type == INFO_ID && info.key.keyString == "n") {
            warningLevel = std::max(warningLevel, 3);
        }
        if (info.key.type == INFO_UNIQUE) {
            warningLevel = std::max(warningLevel, 2);
        }
        if (info.key.type == INFO_WRITE_ONCE) {
            warningLevel = std::max(warningLevel, 1);
        }
    }
    
    QString warning;
    if (warningLevel == 3) {
        warning = tr("<br><br><b>Warning:</b> You cannot change an address's name once it is set!");
    }
    else if (warningLevel == 2) {
        warning = tr("<br><br><b>Warning:</b> You cannot change a unique key once it is set!");
    }
    else if (warningLevel == 1) {
        warning = tr("<br><br><b>Warning:</b> You cannot change a write-once key once it is set!");
    }
    
    fNewRecipientAllowed = false;
    
    QMessageBox::StandardButton retval = QMessageBox::question(
        this,
        tr("Confirm set infos"),
        tr("Are you sure you want to set the following infos on your address %1?<br>%2").arg(
            ui->addressEntry->text()).arg(formatted.join(tr("<br>"))) + warning,
        QMessageBox::Yes|QMessageBox::Cancel,
        QMessageBox::Cancel);
    
    if(retval != QMessageBox::Yes)
    {
        fNewRecipientAllowed = true;
        return;
    }
    
    WalletModel::UnlockContext ctx(model->requestUnlock());
    if(!ctx.isValid())
    {
        // Unlock wallet was cancelled
        fNewRecipientAllowed = true;
        return;
    }
    
    WalletModel::SetInfosReturn infostatus = model->setInfos(ui->addressEntry->text(), infos);
    switch(infostatus.status)
    {
        case WalletModel::InvalidAddress:
            QMessageBox::warning(this, tr("Set Infos"),
                                 tr("The address you provided is not valid, please recheck."),
                                 QMessageBox::Ok, QMessageBox::Ok);
            break;
        case WalletModel::NotOwnAddress:
            QMessageBox::warning(this, tr("Set Infos"),
                                 tr("The address you provided is not one you own, please recheck."),
                                 QMessageBox::Ok, QMessageBox::Ok);
            break;
        case WalletModel::InvalidInfo:
            QMessageBox::warning(this, tr("Set Infos"),
                                 tr("One of the infos you provided is not valid, please recheck."),
                                 QMessageBox::Ok, QMessageBox::Ok);
            break;
        case WalletModel::FailedProcessInfo:
            QMessageBox::warning(this, tr("Set Infos"),
                                 tr("Failed to process an info: %1").arg(infostatus.reason),
                                 QMessageBox::Ok, QMessageBox::Ok);
            break;
        case WalletModel::AmountWithFeeExceedsBalance:
            QMessageBox::warning(this, tr("Set Infos"),
                                 tr("You do not have enough to pay the transaction fees."),
                                 QMessageBox::Ok, QMessageBox::Ok);
            break;
        case WalletModel::TransactionCreationFailed:
            QMessageBox::warning(this, tr("Set Infos"),
                                 tr("Error: Transaction creation failed: %1").arg(infostatus.reason),
                                 QMessageBox::Ok, QMessageBox::Ok);
            break;
        case WalletModel::TransactionCommitFailed:
            QMessageBox::warning(this, tr("Set Infos"),
                                 tr("Error: The transaction was rejected.  This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here."),
                                 QMessageBox::Ok, QMessageBox::Ok);
            break;
        case WalletModel::Aborted: // User aborted, nothing to do
            break;
        case WalletModel::OK:
            accept();
            break;
    }
    fNewRecipientAllowed = true;
}

void AddInfosDialog::on_addressBookButton_clicked()
{
    if(!model)
        return;
    
    AddressBookPage dlg(AddressBookPage::ForSending, AddressBookPage::ReceivingTab, this);
    dlg.setModel(model->getAddressTableModel());
    if(dlg.exec())
    {
        ui->addressEntry->setText(dlg.getReturnValue());
        if (ui->entries->count() <= 0) {
            addEntry();
        }
        ui->entries->itemAt(0)->widget()->setFocus();
    }
}


void AddInfosDialog::clear()
{
    // Remove entries until only one left
    while(ui->entries->count())
    {
        delete ui->entries->takeAt(0)->widget();
    }
    addEntry();
   
    ui->addressEntry->setText("");
    
    updateRemoveEnabled();
    
    ui->submitButton->setDefault(true);
}

void AddInfosDialog::reject()
{
    clear();
}

void AddInfosDialog::accept()
{
    clear();
}

AddInfoEntry *AddInfosDialog::addEntry()
{
    AddInfoEntry *entry = new AddInfoEntry(this);
    entry->setModel(model);
    ui->entries->addWidget(entry);
    connect(entry, SIGNAL(removeEntry(AddInfoEntry*)), this, SLOT(removeEntry(AddInfoEntry*)));
    
    updateRemoveEnabled();
    
    // Focus the field, so that entry can start immediately
    entry->clear();
    entry->setFocus();
    ui->scrollAreaWidgetContents->resize(ui->scrollAreaWidgetContents->sizeHint());
    QCoreApplication::instance()->processEvents();
    QScrollBar* bar = ui->scrollArea->verticalScrollBar();
    if (bar)
        bar->setSliderPosition(bar->maximum());
    return entry;
}

void AddInfosDialog::updateRemoveEnabled()
{
    // Remove buttons are enabled as soon as there is more than one send-entry
    bool enabled = (ui->entries->count() > 1);
    for(int i = 0; i < ui->entries->count(); ++i)
    {
        AddInfoEntry *entry = qobject_cast<AddInfoEntry*>(ui->entries->itemAt(i)->widget());
        if(entry)
        {
            entry->setRemoveEnabled(enabled);
        }
    }
    setupTabChain(0);
}

void AddInfosDialog::removeEntry(AddInfoEntry* entry)
{
    delete entry;
    updateRemoveEnabled();
}

QWidget *AddInfosDialog::setupTabChain(QWidget *prev)
{
    QWidget::setTabOrder(prev, ui->addressEntry);
    prev = ui->addressEntry;
    for(int i = 0; i < ui->entries->count(); ++i)
    {
        AddInfoEntry *entry = qobject_cast<AddInfoEntry*>(ui->entries->itemAt(i)->widget());
        if(entry)
        {
            prev = entry->setupTabChain(prev);
        }
    }
    QWidget::setTabOrder(prev, ui->addButton);
    QWidget::setTabOrder(ui->addButton, ui->submitButton);
    return ui->submitButton;
}

void AddInfosDialog::pasteEntry(const CTxInfo &rv)
{
    if (!fNewRecipientAllowed)
        return;
    
    AddInfoEntry *entry = 0;
    // Replace the first entry if it is still unused
    if(ui->entries->count() == 1)
    {
        AddInfoEntry *first = qobject_cast<AddInfoEntry*>(ui->entries->itemAt(0)->widget());
        if(first->isClear())
        {
            entry = first;
        }
    }
    if(!entry)
    {
        entry = addEntry();
    }
    
    entry->setValue(rv);
}
