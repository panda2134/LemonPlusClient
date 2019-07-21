#ifndef LEMONPLUSCONNECTION_H
#define LEMONPLUSCONNECTION_H

#include "logger.h"
#include "quazip/JlCompress.h"

#include <QDir>
#include <QTimer>
#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QDataStream>
#include <QHostAddress>
#include <QSharedPointer>
#include <QTemporaryFile>
#include <QRandomGenerator>

const qint64 BYTES_PER_PACKET = 1024;
const int COMPRESSION_RETRY_TIMES = 10;
const int COMPRESSION_RETRY_WAIT = 30;

class LemonPlusConnection : public QObject
{
    Q_OBJECT
public:
    LemonPlusConnection(QObject *, QPair<QString, QHostAddress> &, QDir);
    ~LemonPlusConnection();

    Logger *logger = nullptr;
signals:
    void setProgress(int progress);

public slots:
    void startConnection();

private:
    const QString uniqueId = "LEMONPLUS";
    bool online = false;
    QSharedPointer<QTimer> connectionTimer;
    QPair<QString, QHostAddress> server;
    QDir dir;
    QString tmpZipName;
    QTcpSocket *tcp = nullptr;
    QDataStream *stream = nullptr;
    void readCmd();

private slots:
    void doRegister();
    void checkConnection();
    void onReadyRead();
    void onTcpError(QAbstractSocket::SocketError);
};

#endif // LEMONPLUSCONNECTION_H
