#include "lemonplusclient.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator qtTranslator, appTranslator;

    qtTranslator.load(QString(":/translation/qt_zh_CN.qm"));
    appTranslator.load(QString(":/translation/lemonplusclient_zh_CN.qm"));

    a.installTranslator(&qtTranslator);
    a.installTranslator(&appTranslator);

    LemonPlusClient w;
    w.show();

    return a.exec();
}
