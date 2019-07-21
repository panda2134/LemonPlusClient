#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>

class Logger : public QObject
{
    Q_OBJECT
public:
    enum LoggingLevel {
        INFO, WARNING, ERROR, FATAL, SUCCESS
    };
    explicit Logger(QObject *parent = nullptr);

signals:
    void log(const QString &value, int level);
};

#endif // LOGGER_H
