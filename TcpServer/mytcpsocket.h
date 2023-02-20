#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include "protocol.h"
#include "opedb.h"
#include <QFile>
#include <QTimer>

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT

public:
    MyTcpSocket();
    QString getName();

public slots:
    void recvMsg();
    void clientOffline();//处理下线
    void sendFileToClient();//发送文件给客户端
    void copyDir(QString strSrcDir, QString strDestDir);//拷贝文件夹


signals:
    void offline(MyTcpSocket * m_socket);

private:
    QString m_strName;//用户名

    QFile m_file;//接收的文件
    qint64 m_iTotal;//文件大小
    qint64 m_iRecved;//文件接收进度
    bool m_bUpload;//用户是否为上传状态
    QTimer *m_pTimer;//定时器
};

#endif // MYTCPSOCKET_H
