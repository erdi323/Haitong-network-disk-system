#include "tcpserver.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TcpServer w;
    OpeDB::getInstance().init();
    OpeDB::getInstance().serverOffline();//全部下线
    w.show();
    return a.exec();
}
