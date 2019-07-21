#ifndef LEMONPLUSCLIENT_H
#define LEMONPLUSCLIENT_H

#include <QMainWindow>

#include "logger.h"
#include "lemonplusconnection.h"
#include "lanbroadcastreceiver.h"

typedef QPair<QString, QHostAddress> HostInfo;
Q_DECLARE_METATYPE(HostInfo)

namespace Ui {
class LemonPlusClient;
}

class LemonPlusClient : public QMainWindow
{
    Q_OBJECT

public:
    explicit LemonPlusClient(QWidget *parent = 0);
    ~LemonPlusClient();
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::LemonPlusClient *ui;
    int networkInterfaceIndex;
    LanBroadcastReceiver *recv;
    LemonPlusConnection *conn = nullptr;
    QThread *recvThread, *connThread;
    Logger *logger;

    void welcome();
    void setUiInteract(bool);

private slots:
    void doLogging(const QString &value, int level);
    void settingsAction();
    void aboutLemonPlusClient();
    void updateServerList();
    void editFolderPath();
    void doConnection();
    void onConnectionFinished();
};

#endif // LEMONPLUSCLIENT_H
