#include "lemonplusconnection.h"

LemonPlusConnection::LemonPlusConnection(QObject *parent, QPair<QString, QHostAddress> &srv, QDir dir): QObject(parent)
{
    logger = new Logger();
    this->server = srv;
    this->dir = dir;
}

LemonPlusConnection::~LemonPlusConnection()
{
    if(logger != nullptr) delete logger;
    if(tcp != nullptr) delete tcp;
    if(stream != nullptr) delete stream;
}

void LemonPlusConnection::startConnection()
{
    tcp = new QTcpSocket();
    tcp->connectToHost(server.second, 53667);

    connectionTimer = static_cast<QSharedPointer<QTimer> >(new QTimer());
    connectionTimer->setSingleShot(true);
    connectionTimer->setInterval(4980);
    connectionTimer->start();
    connect(&(*connectionTimer), SIGNAL(timeout()), this, SLOT(checkConnection()));

    stream = new QDataStream(tcp);
    stream->setVersion(QDataStream::Qt_5_7);

    connect(tcp, SIGNAL(connected()), this, SLOT(doRegister()));

    connect(tcp, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(tcp, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onTcpError(QAbstractSocket::SocketError)));
}

void LemonPlusConnection::doRegister()
{
    (*stream) << uniqueId << QString("REGISTER") << dir.dirName();
    tcp->flush();
}

void LemonPlusConnection::checkConnection()
{
    if(!online) {
        emit logger->log(tr("Cannot connect to the server. Maybe the contest hasn't been loaded."), Logger::FATAL);
        tcp->disconnectFromHost();
        this->thread()->quit();
    }
}

void LemonPlusConnection::onReadyRead()
{
    while(true) {
        stream->startTransaction();
        this->readCmd();
        if(stream->commitTransaction()) {
            qDebug() << "commited";
        } else break;
    }
}

void LemonPlusConnection::readCmd()
{
    QString id, cmd;
    (*stream) >> id >> cmd;
    if(stream->status() != stream->Ok) return;
    qDebug() << "SERVER CMD : " << id << cmd;
    if(id != uniqueId) {
        online = false;
        emit logger->log(tr("Client cannot understand the protocal used by this server."), Logger::FATAL);
        tcp->disconnectFromHost();
        this->thread()->quit();
        return;
    }
    if(cmd == "OK_CONNECTION_ESTABLISHED") {
        if(online) {
            emit logger->log(tr("Server is attempting to establish more than one connection."), Logger::WARNING);
            return;
        } else {
            online = true;
            emit logger->log(tr("Connected to Lemon+ contest server."), Logger::SUCCESS);
            return;
        }
    } else if(cmd == "ERR_NAME_USED") {
        online = false;
        emit logger->log(tr("Contestant name already used.") + " " + tr("Try renaming the contestant folder."), Logger::FATAL);
        tcp->disconnectFromHost();
        this->thread()->quit();
        return;
    } else if(cmd == "ERR_INVALID_NAME") {
        online = false;
        emit logger->log(tr("Contestant name invalid.") + " " + tr("Try renaming the contestant folder."), Logger::FATAL);
        tcp->disconnectFromHost();
        this->thread()->quit();
        return;
    } else if(cmd == "OK_GOT_CODE") {
        emit logger->log(tr("The source code folder has been successfully uploaded."), Logger::SUCCESS);
        return;
    } else if(cmd == "COLLECT_CODE") {
        // TODO:
        // 1. zip
        // 2. fragment size = 1024byte
        // 3. upload per fragment; size:
        // [size] -1 -1 -1 ...
        emit logger->log(tr("Server has requested the client to upload the contestant's code."), Logger::INFO);
        emit logger->log(tr("Compressing the code..."), Logger::INFO);

        emit setProgress(0);

        tmpZipName = QDir::toNativeSeparators(QDir::tempPath()) + QDir::separator()
                + QString("LemonPlusClient_ContestantCode_%1").arg(QRandomGenerator::system()->generate())
                + ".zip";
        {
            int i;
            for(i = 0; i < COMPRESSION_RETRY_TIMES; i++) {
                if(!JlCompress::compressDir(tmpZipName, dir.absolutePath())) {
                    emit logger->log(tr("Compression failed! Retry in %1 seconds... Close any program that uses the files.")
                                     .arg(COMPRESSION_RETRY_WAIT), Logger::ERROR);
                    this->thread()->sleep(COMPRESSION_RETRY_WAIT);
                } else break;
            }
            if(i == COMPRESSION_RETRY_TIMES) {
                emit logger->log(tr("All attempts of compression failed!"), Logger::FATAL);
                tcp->disconnectFromHost();
                this->thread()->quit();
                return;
            }
            emit logger->log(tr("Source code compressed."), Logger::INFO);
        }

        qDebug() << tmpZipName;
        QSharedPointer<QFile> tmpZip =
                QSharedPointer<QFile>(new QFile(tmpZipName));
        bool firstPacket = true;
        qint64 bytesWritten = 0, tmpZipSize = 0;

        tmpZip->open(QIODevice::ReadOnly);
        tmpZipSize = tmpZip->size();
        tmpZip->seek(0);

        emit logger->log(tr("Compressed file contains %1 bytes.").arg(tmpZipSize), Logger::INFO);
        tcp->flush();

        QByteArray buf;
        while((buf = tmpZip->read(BYTES_PER_PACKET)).size() != 0) {
            (*stream) << uniqueId << QString("CODE_PAYLOAD");
            if(firstPacket) {
                (*stream) << tmpZipSize;
                firstPacket = false;
                emit logger->log(tr("Now uploading files..."), Logger::INFO);
            } else {
                (*stream) << qint64(-1);
            }
            (*stream) << buf;
            if(stream->status() == QDataStream::Ok) {
                // ok
                tcp->flush();
                bytesWritten += buf.size();
                buf.clear();
                emit setProgress(bytesWritten * 100 / tmpZipSize);
            } else {
                emit setProgress(0);
                emit logger->log(tr("Failed to write data at %1 byte. Broken pipe.")
                                 .arg(tmpZip->pos()), Logger::FATAL);
                tcp->disconnectFromHost();
                this->thread()->quit();
                return;
            }
        }

        tcp->flush(); // flush again to avoid failure
        tmpZip->close();
        emit logger->log(tr("The contestant's code has been sent to the server. Waiting for reply..."), Logger::INFO);
    }
}

void LemonPlusConnection::onTcpError(QAbstractSocket::SocketError)
{
    online = false;
    emit logger->log(tr("TCP Error! Broken pipe. Maybe the server has closed the connection."), Logger::FATAL);
    tcp->disconnectFromHost();
    this->thread()->quit();
}
