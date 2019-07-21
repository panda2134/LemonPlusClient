#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QNetworkInterface>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    auto ifList = QNetworkInterface::allInterfaces();
    for (auto interface : ifList)
    {
        ui->networkInterface->addItem(interface.humanReadableName());
    }

    connect(ui->networkInterface, SIGNAL(currentIndexChanged(int)), this, SLOT(networkInterfaceChanged()));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

int SettingsDialog::getNetworkInterfaceIndex() const
{
    return networkInterfaceIndex;
}

void SettingsDialog::setNetworkInterfaceIndex(int value)
{
    networkInterfaceIndex = value;
    if(0 <= value && value < ui->networkInterface->count())
        ui->networkInterface->setCurrentIndex(value);
}

void SettingsDialog::networkInterfaceChanged()
{
    networkInterfaceIndex = ui->networkInterface->currentIndex();
}
