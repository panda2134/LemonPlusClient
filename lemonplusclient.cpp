#include "lemonplusclient.h"
#include "ui_lemonplusclient.h"
#include "settingsdialog.h"
#include "logger.h"

#include <QSettings>
#include <QDateTime>
#include <QMessageBox>
#include <QThread>
#include <QNetworkInterface>
#include <QFileDialog>

LemonPlusClient::LemonPlusClient(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LemonPlusClient)
{
    ui->setupUi(this);


    QSettings settings("panda_2134", "LemonPlusClient");
    this->networkInterfaceIndex = settings.value("networkInterfaceIndex", 0).toInt();

    recv = new LanBroadcastReceiver();
    recvThread = new QThread();
    recv->setInterface(QNetworkInterface::allInterfaces().at(networkInterfaceIndex));
    recv->moveToThread(recvThread);
    connect(recv->getLogger(), SIGNAL(log(QString, int)), this, SLOT(doLogging(QString, int)));
    connect(recv, SIGNAL(serverUpdated()), this, SLOT(updateServerList()));
    connect(recvThread, SIGNAL(started()), recv, SLOT(startReceivingBroadcast()));
    connect(recvThread, SIGNAL(finished()), recv, SLOT(deleteLater()));
    recvThread->start();

    logger = new Logger();
    connect(logger, SIGNAL(log(QString,int)), this, SLOT(doLogging(QString,int)));

    connect(ui->actionSettings, SIGNAL(triggered(bool)), this, SLOT(settingsAction()));
    connect(ui->actionExit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(aboutLemonPlusClient()));

    connect(ui->fileDialogButton, SIGNAL(clicked(bool)), this, SLOT(editFolderPath()));
    connect(ui->connectPushButton, SIGNAL(clicked(bool)), this, SLOT(doConnection()));

    this->welcome();
}

LemonPlusClient::~LemonPlusClient()
{
    delete ui;
    delete logger;
}

void LemonPlusClient::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    QSettings settings("panda_2134", "LemonPlusClient");
    settings.setValue("networkInterfaceIndex", networkInterfaceIndex);
}

void LemonPlusClient::welcome()
{
    emit logger->log(tr("Welcome you to use LemonPlus Client! To start, do the configuration above, and then click \"Connect\"."), Logger::INFO);
}

void LemonPlusClient::setUiInteract(bool val)
{
    ui->serverComboBox->setEnabled(val);
    ui->folderPathEdit->setEnabled(val);
    ui->fileDialogButton->setEnabled(val);
    ui->connectPushButton->setEnabled(val);
}

void LemonPlusClient::doLogging(const QString &value, int level)
{
    QString curTime = QString("[") + QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate) + QString("]");
    QString html = curTime + " " + value;
    switch(level) {
    case Logger::INFO:
        ui->logText->appendHtml(html);
    break;
    case Logger::WARNING:
        ui->logText->appendHtml(QString("<span style=\"color: orange\">") + html + "</span>");
    break;
    case Logger::ERROR:
    case Logger::FATAL:
        ui->logText->appendHtml(QString("<span style=\"color: red\">") + html + "</span>");
    break;
    case Logger::SUCCESS:
        ui->logText->appendHtml(QString("<span style=\"color: green\">" + html + "</span>"));
    break;
    }
}

void LemonPlusClient::settingsAction()
{
    SettingsDialog *dialog = new SettingsDialog;
    dialog->setNetworkInterfaceIndex(this->networkInterfaceIndex);

    if(dialog->exec() == QDialog::Accepted) {
        this->networkInterfaceIndex = dialog->getNetworkInterfaceIndex();
        recv->setInterface(QNetworkInterface::allInterfaces().at(networkInterfaceIndex));
    }
}

void LemonPlusClient::aboutLemonPlusClient()
{
    QString text;
    text += "<h2>LemonPlus Client</h2>";
    text += tr("A tool for programming contest code collection.") + "<br>";
    text += tr("Build Date: %1").arg(__DATE__) + "<br>";
    text += tr("This program is under the <a href=\"http://www.gnu.org/licenses/gpl-3.0.html\">GPLv3</a> license")
            + "<br>";
    text += tr("by panda2134") + "<br>";
    QMessageBox::about(this, "LemonPlus Client", text);
}

void LemonPlusClient::updateServerList()
{
    ui->serverComboBox->clear();
    for (HostInfo x : recv->getServer()) {
        QString text;
        text += x.first; text += " ";
        text += QString("(") + x.second.toString() + QString(")");
        ui->serverComboBox->addItem(text, QVariant::fromValue(x));
    }
    ui->connectPushButton->setEnabled(ui->serverComboBox->count() != 0);
}

void LemonPlusClient::editFolderPath()
{
    ui->folderPathEdit->setText(QFileDialog::getExistingDirectory());
}

void LemonPlusClient::doConnection()
{
    if(ui->folderPathEdit->text().count() == 0 || !QDir(ui->folderPathEdit->text()).exists()) {
        emit logger->log(tr("Contestant folder not found."), Logger::ERROR);
        QMessageBox::critical(this, tr("Invalid folder path"), tr("Contestant folder not found."));
        return;
    }

    emit logger->log(tr("Connecting..."), Logger::INFO);
    setUiInteract(false);

    HostInfo hostInfo = ui->serverComboBox->itemData(ui->serverComboBox->currentIndex()).value<HostInfo>();
    conn = new LemonPlusConnection(nullptr, hostInfo, QDir(ui->folderPathEdit->text()));
    connThread = new QThread();
    conn->moveToThread(connThread);
    connect(conn->logger, SIGNAL(log(QString,int)), this, SLOT(doLogging(QString,int)));
    connect(connThread, SIGNAL(started()), conn, SLOT(startConnection()));
    connect(connThread, SIGNAL(finished()), conn, SLOT(deleteLater()));
    connect(connThread, SIGNAL(finished()), this, SLOT(onConnectionFinished()));

    ui->progressBar->setMaximum(100);
    connect(conn, SIGNAL(setProgress(int)), ui->progressBar, SLOT(setValue(int)));

    qDebug() << hostInfo;
    connThread->start();
}

void LemonPlusClient::onConnectionFinished()
{
    qDebug() << "connection ended";
    setUiInteract(true);
}
