#ifndef TCPCLIENT_H
#define TCPCLIENT_H

//客户端

#include <QWidget>
#include <QFile>
#include <QTcpSocket>//tcp网络通信接口 需在.pro文件添加QT += network
#include "protocol.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TcpClient; }
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();

    void loadConfig();//加载配置文件

    static TcpClient &getInstance();
    QTcpSocket& getTcpSocket();
    QString &loginName();
    QString &curPath();
    void setCurPath(QString strCurPath);//修改当前目录


public slots:
    void showConnect();//显示Tcp连接状态

    void recvMsg();

private slots:
   // void on_pushButton_sendPd_clicked();

    void on_pushButton_login_clicked();

    void on_pushButton_regist_clicked();

    void on_pushButton_revoked_clicked();

private:
    Ui::TcpClient *ui;
    QString m_strIP;//ip地址
    quint16 m_usPort;//端口
    QTcpSocket m_tcpSocket;//连接服务器，和服务器数据交互
    QString m_strLoginName;//登录的用户名
    QString m_strCurPath;//用户当前文件路径
    QFile m_file;//下载文件

};
#endif // TCPCLIENT_H
