#include "tcpclient.h"

#include <QApplication>
//#include "sharefile.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //ShareFile w; w.show();
    TcpClient::getInstance().show();
    return a.exec();
}
