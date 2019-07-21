#ifndef LANBROADCASTRECEIVER_H
#define LANBROADCASTRECEIVER_H

#include "logger.h"

#include <QPair>
#include <QDebug>
#include <QObject>
#include <QThread>
#include <QUdpSocket>
#include <QDataStream>
#include <QNetworkInterface>

class LanBroadcastReceiver : public QObject
{
    Q_OBJECT
public:
    explicit LanBroadcastReceiver(QObject *parent = nullptr);
    ~LanBroadcastReceiver();
    QNetworkInterface getInterface() const;
    void setInterface(const QNetworkInterface &value);

    const QSet<QPair<QString, QHostAddress> >& getServer() const;

    const Logger* getLogger() const;

signals:
    void serverUpdated();
public slots:
    void startReceivingBroadcast();
private:
    QUdpSocket *udp4, *udp6;
    Logger logger;
    const int retrySecond = 5;
    const QHostAddress grpAddr4 = QHostAddress("224.0.36.166");
    const QHostAddress grpAddr6 = QHostAddress("ff18::3666");
    QNetworkInterface interface;
    QSet<QPair<QString, QHostAddress> > server;
    void processBroadcastBySocket(QUdpSocket *udp);
private slots:
    void broadcastReceived();
};

#endif // LANBROADCASTRECEIVER_H
