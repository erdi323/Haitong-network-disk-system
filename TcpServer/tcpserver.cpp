#include "tcpserver.h"
#include "ui_tcpserver.h"
#include <QByteArray>
#include <QMessageBox>
#include <QHostAddress>

TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);

    loadConfig();
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP),m_usPort);//监听

}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    QFile file(":/server.config");
    if (file.open(QIODevice::ReadOnly))//只读
    {
        QByteArray badata = file.readAll();//读取全部信息
        QString strData = badata.toStdString().c_str();//转换成字符串
        file.close();//关闭数据库

        strData.replace("\r\n" ," ");//将\r\n替换成空格
        QStringList strList = strData.split(" ");//切分
        m_strIP = strList[0];
        m_usPort = strList[1].toUShort();
        log << "ip: " << m_strIP
            << "port: " << m_usPort;
    }
    else
    {
        QMessageBox::critical(this, "open config", "open critical");//弹窗报错
    }
}
