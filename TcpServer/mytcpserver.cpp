#include "mytcpserver.h"


MyTcpServer::MyTcpServer()
{

}

MyTcpServer& MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return  instance;
}


//重写传入连接函数
void MyTcpServer::incomingConnection(qintptr handle)
{
    log << "新客户端已连接new client connected";

    MyTcpSocket *pTcpSocket = new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(handle);//设置套接字描述符
    m_tcpSocketlis.append(pTcpSocket);//将值列表的项追加到列表。
    QObject::connect(pTcpSocket,SIGNAL(offline(MyTcpSocket *)),//&MyTcpSocket::offline,
                     this,SLOT(deleteSocket(MyTcpSocket *)));//&MyTcpServer::deleteSocket);
}


//转发
void MyTcpServer::resend(const char *pername, PDU *pdu)
{
    if(NULL == pername || pdu ==NULL)
    {
        log;
        return;
    }
    QString strName = pername;
    for(int i = 0; i < m_tcpSocketlis.size(); i++)
    {
        if(m_tcpSocketlis.at(i)->getName() == strName)
        {
            log;
            m_tcpSocketlis.at(i)->write((char*)pdu,pdu->uiPDULen);
            break;
        }
    }
    log;
}

void MyTcpServer::deleteSocket(MyTcpSocket *m_socket)//处理下线
{
    log;
    QList<MyTcpSocket*>::iterator iter = m_tcpSocketlis.begin();
    for(; iter != m_tcpSocketlis.end(); iter++)
    {
        if(m_socket == *iter)
        {
            //delete *iter;//这个会在任意一个客户端下线30秒后使服务器端闪退
            (*iter)->deleteLater();//释放空间
            *iter = NULL;
            m_tcpSocketlis.erase(iter);//移除
            log;
            break;
        }
    }
    for(int i = 0; i < m_tcpSocketlis.size(); i++)//显示在线成员
    {
        log << m_tcpSocketlis.at(i)->getName();
    }
}

