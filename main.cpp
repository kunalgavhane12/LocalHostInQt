#include <QCoreApplication>
#include "webserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    WebServer webserver;
    webserver.start();

    return a.exec();
}
