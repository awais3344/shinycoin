#include "addinfoentry.h"
#include "ui_addinfoentry.h"
#include "guiutil.h"
#include "bitcoinunits.h"
#include "addressbookpage.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "addresstablemodel.h"

#include <QApplication>
#include <QClipboard>

AddInfoEntry::AddInfoEntry(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::AddInfoEntry),
    model(0)
{
    ui->setupUi(this);

    setFocusPolicy(Qt::TabFocus);
    
    connect(ui->keyType, SIGNAL(currentIndexChanged(int)), this, SLOT(keyTypeChanged(int)));
    // Initialize the placeholders & focus proxies
    keyTypeChanged(0);
}

void AddInfoEntry::keyTypeChanged(int index) {
    if (ui->keyType->currentText() == "Name") {
        ui->keyString->setEnabled(false);
        ui->keyString->setText("");
        ui->valueString->setFocus();
        setFocusProxy(ui->valueString);
#if QT_VERSION >= 0x040700
        ui->keyString->setPlaceholderText("");
        ui->valueString->setPlaceholderText(tr("Enter the desired address name (lowercase letters, numbers, and dashes only)"));
#endif
    }
    else {
        ui->keyString->setEnabled(true);
        ui->keyString->setFocus();
        setFocusProxy(ui->keyString);
#if QT_VERSION >= 0x040700
        ui->keyString->setPlaceholderText(tr("Enter the key string (lowercase letters, numbers, and dashes only)"));
        if (currentKeyType() == INFO_ID) {
            ui->valueString->setPlaceholderText(tr("Enter the id value (lowercase letters, numbers, and dashes only)"));
        }
        else {
            ui->valueString->setPlaceholderText(tr("Enter the value (no restrictions)"));
        }
#endif
    }
}

AddInfoEntry::~AddInfoEntry()
{
    delete ui;
}

void AddInfoEntry::setModel(WalletModel *model)
{
    this->model = model;
    clear();
}

void AddInfoEntry::setRemoveEnabled(bool enabled)
{
    ui->deleteButton->setEnabled(enabled);
}

void AddInfoEntry::clear()
{
    ui->keyString->clear();
    ui->valueString->clear();
    ui->keyType->setCurrentIndex(0);
}

void AddInfoEntry::on_deleteButton_clicked()
{
    emit removeEntry(this);
}

bool AddInfoEntry::validate()
{
    CTxInfo info = getValue();
    
    if (!info.key.IsValid()) {
        ui->keyString->setValid(false);
        return false;
    }
    if (!info.IsValid()) {
        ui->valueString->setValid(false);
        return false;
    }
    
    return true;
}

TxInfoType AddInfoEntry::currentKeyType() const
{
    QString typeStr = ui->keyType->currentText();
    
    if (typeStr == "Name") {
        return INFO_ID;
    }
    else if (typeStr == "Normal") {
        return INFO_NORMAL;
    }
    else if (typeStr == "Write-Once") {
        return INFO_WRITE_ONCE;
    }
    else if (typeStr == "Unique") {
        return INFO_UNIQUE;
    }
    else if (typeStr == "Id") {
        return INFO_ID;
    }
    else {
        throw new std::runtime_error("Unexpected type string");
    }
}

CTxInfo AddInfoEntry::getValue()
{
    QString typeStr = ui->keyType->currentText();
    std::string keyString = ui->keyString->text().toAscii().data();
    
    TxInfoKey key;
    if (typeStr == "Name") {
        key = TxInfoKey(INFO_ID, "n");
    }
    else {
        key = TxInfoKey(currentKeyType(), keyString);
    }
    
    return CTxInfo(key, ui->valueString->text().toAscii().data());
}

QWidget *AddInfoEntry::setupTabChain(QWidget *prev)
{
    QWidget::setTabOrder(prev, ui->keyType);
    QWidget::setTabOrder(ui->keyType, ui->keyString);
    QWidget::setTabOrder(ui->keyString, ui->valueString);
    return ui->valueString;
}

void AddInfoEntry::setValue(const CTxInfo &value)
{
}

bool AddInfoEntry::isClear()
{
    return ui->valueString->text().isEmpty();
}

void AddInfoEntry::setFocus()
{
    ui->valueString->setFocus();
}

