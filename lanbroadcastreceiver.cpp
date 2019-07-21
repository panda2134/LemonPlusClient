#include "lanbroadcastreceiver.h"

LanBroadcastReceiver::LanBroadcastReceiver(QObject *parent) : QObject(parent)
{
}

LanBroadcastReceiver::~LanBroadcastReceiver()
{
    delete udp4;
    delete udp6;
}

QNetworkInterface LanBroadcastReceiver::getInterface() const
{
    return interface;
}

void LanBroadcastReceiver::setInterface(const QNetworkInterface &value)
{
    interface = value;
}

void LanBroadcastReceiver::startReceivingBroadcast() {
    udp4 = new QUdpSocket(), udp6 = new QUdpSocket();

    udp4->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 1);
    udp6->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 1);

    bool ok4 = true, ok6 = true;

    while (true) {
        emit logger.log(tr("Joining IPv4 multicast group..."), Logger::INFO);
        ok4 &= udp4->bind(QHostAddress::AnyIPv4, 53666, QUdpSocket::ShareAddress);
        ok4 &= udp4->joinMulticastGroup(grpAddr4, interface);
        if(ok4) emit logger.log(tr("Succeeded."), Logger::INFO);
        else emit logger.log(tr("Failed."), Logger::WARNING);

        emit logger.log(tr("Joining IPv6 multicast group..."), Logger::INFO);
        ok6 &= udp6->bind(QHostAddress::AnyIPv6, 53666, QUdpSocket::ShareAddress);
        ok6 &= udp6->joinMulticastGroup(grpAddr6, interface);
        if(ok6) emit logger.log(tr("Succeeded."), Logger::INFO);
        else emit logger.log(tr("Failed."), Logger::WARNING);

        if(ok4 || ok6) break;
        else {
            emit logger.log(tr("Cannot join any multicast group. Retry in %1 seconds...").arg(retrySecond), Logger::ERROR);
            this->thread()->sleep(retrySecond);
        }
    }

    connect(udp4, SIGNAL(readyRead()), this, SLOT(broadcastReceived()));
    connect(udp6, SIGNAL(readyRead()), this, SLOT(broadcastReceived()));
}

void LanBroadcastReceiver::processBroadcastBySocket(QUdpSocket *udp) {
    QByteArray datagram;
    while(udp->hasPendingDatagrams()) {
        datagram.resize(udp->pendingDatagramSize());
        udp->readDatagram(datagram.data(), datagram.size());

        QDataStream stream(&datagram, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_5_7);
        QString uniqueId, serverName;
        QHostAddress serverAddr;

        stream.startTransaction();
        stream >> uniqueId >> serverName >> serverAddr;
        if(!stream.commitTransaction()) return;

        if(uniqueId != "LEMONPLUS")
            continue;
        server.insert(qMakePair(serverName, serverAddr));
    }
}

void LanBroadcastReceiver::broadcastReceived()
{
    int lastSize = server.size();
    if(udp4->state() == QUdpSocket::BoundState)
        processBroadcastBySocket(udp4);
    if(udp6->state() == QUdpSocket::BoundState)
        processBroadcastBySocket(udp6);
    if(server.size() != lastSize)
        emit serverUpdated();
}

const Logger* LanBroadcastReceiver::getLogger() const
{
    return &logger;
}

const QSet<QPair<QString, QHostAddress> >& LanBroadcastReceiver::getServer() const
{
    return server;
}
