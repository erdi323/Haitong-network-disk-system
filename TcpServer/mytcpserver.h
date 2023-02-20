#ifndef MYQTCPSERVER_H
#define MYQTCPSERVER_H

#include "utils.h"
#include <QTcpServer>//tcp协议通信的服务端类 需在.pro文件添加QT += network
#include <QList>//列表类
#include "mytcpsocket.h"

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
private:
    QList<MyTcpSocket*> m_tcpSocketlis;

    MyTcpServer();

public slots:
    void deleteSocket(MyTcpSocket * m_socket);


public:
    static MyTcpServer& getInstance();//获得实例

    virtual void incomingConnection(qintptr handle);//重写传入连接

    void resend(const char* pername, PDU *pdu);//转发

};

#endif // MYQTCPSERVER_H
