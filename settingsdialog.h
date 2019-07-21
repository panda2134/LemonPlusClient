#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    int getNetworkInterfaceIndex() const;
    void setNetworkInterfaceIndex(int value);

private:
    Ui::SettingsDialog *ui;
    int networkInterfaceIndex;

private slots:
    void networkInterfaceChanged();
};

#endif // SETTINGSDIALOG_H
