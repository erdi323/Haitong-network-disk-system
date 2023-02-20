#ifndef TCPSERVER_H
#define TCPSERVER_H

//服务器端

#include <QWidget>
#include "utils.h"
#include "mytcpserver.h"
#include "mytcpsocket.h"
#include <QFile>
#include "opedb.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TcpServer; }
QT_END_NAMESPACE

class TcpServer : public QWidget
{
    Q_OBJECT

public:
    TcpServer(QWidget *parent = nullptr);
    ~TcpServer();

    void loadConfig();//加载配置文件

private:
    Ui::TcpServer *ui;
    QString m_strIP;//ip地址
    quint16 m_usPort;//端口

};
#endif // TCPSERVER_H
