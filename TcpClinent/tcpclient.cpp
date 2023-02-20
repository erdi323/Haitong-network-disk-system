#include "tcpclient.h"
#include "ui_tcpclient.h"
#include "utils.h"
#include <QByteArray>
#include <QMessageBox>
#include <QHostAddress>
#include "opewidget.h"
#include "privatechat.h"
#include <QDir>
TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);

    resize(400,200);
    loadConfig();
    //连接
    QObject::connect(&m_tcpSocket,&QTcpSocket::connected,
                     this,&TcpClient::showConnect);

    QObject::connect(&m_tcpSocket,&QTcpSocket::readyRead,
                     this,&TcpClient::recvMsg);

    //QObject::connect(&m_tcpSocket,&QTcpSocket::disconnected, this,&TcpClient::recvMsg)


    m_tcpSocket.connectToHost(QHostAddress(m_strIP),m_usPort);//连接服务器 传入（地址，端口）

}

TcpClient::~TcpClient()
{
    delete ui;
}

//加载配置文件
void TcpClient::loadConfig()
{
    QFile file(":/client.config");
    if (file.open(QIODevice::ReadOnly))//只读
    {
        QByteArray badata = file.readAll();//读取全部信息
        QString strData = badata.toStdString().c_str();//转换成字符串
        file.close();//关闭数据库

        strData.replace("\r\n" ," ");//将\r\n替换成空格
        QStringList strList = strData.split(" ");//切分
        m_strIP = strList[0];
        m_usPort = strList[1].toUShort();

    }
    else
    {
        QMessageBox::critical(this, "open config", "open critical");//弹窗报错
    }
}

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}

QString &TcpClient::loginName()
{
    return m_strLoginName;
}

QString &TcpClient::curPath()
{
    return m_strCurPath;
}

void TcpClient::setCurPath(QString strCurPath)//修改当前目录
{
    m_strCurPath = strCurPath;
}

void TcpClient::showConnect()
{
    QMessageBox::information(this, "连接服务器", "连接服务器成功");//弹窗
}
/*
void TcpClient::on_pushButton_sendPd_clicked()
{
    QString strMsg = ui->lineEdit->text();
    if (!strMsg.isEmpty())
    {
        PDU *pdu = mkPDU(strMsg.size() + 1);//声明构造
        pdu->uiMsgType = 8888;//临时给予类型

        //c_str ()函数返回一个指向正规C字符串的 指针, 内容与本string串相同.
        memcpy(pdu->caMsg,strMsg.toStdString().c_str(),strMsg.size());//拷贝数据进入结构体
        log << (char*)pdu->caMsg;
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);//发送
        free(pdu);//释放
        pdu = NULL;
    }
    else
    {
        QMessageBox::warning(this, "信息发送", "发送的信息不能为空");//弹窗报错
    }
}*/

void TcpClient::recvMsg()//接收信息
{
    if(!OpeWidget::getInstance().getBook()->getDownloadStatus())//判断是否为下载状态
    {
        log << m_tcpSocket.bytesAvailable();
        uint uiPDULen = 0;
        m_tcpSocket.read((char*)&uiPDULen, sizeof(uint));//获得信息总大小
        uint uiMsgLen = uiPDULen-sizeof (PDU);
        log << uiMsgLen;
        PDU *pdu = mkPDU(uiMsgLen);
        m_tcpSocket.read((char*)pdu + sizeof (uint),uiPDULen - sizeof (uint));//读取剩余消息数据
        log << pdu->uiMsgType << " "<< pdu->caMsg << " "<< pdu->caData;
        switch (pdu->uiMsgType)
        {
        case ENUM_MSG_TYPE_REGIST_RESPOND://注册回复2
        {
            log << "注册回复";
            if (0 == strcmp(pdu->caData,REGIST_OK))
            {
                QMessageBox::information(this, "注册", "注册成功");
            }
            else if (0 == strcmp(pdu->caData,REGIST_FAILED))
            {
                QMessageBox::warning(this, "注册", "注册失败");
            }
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_RESPOND://登录回复4
        {
            log << "登录";
            if (0 == strcmp(pdu->caData,LOGIN_OK))
            {
                QMessageBox::information(this, "登录", "登录成功");
                m_strCurPath = QString("./%1").arg(m_strLoginName);
                OpeWidget::getInstance().show();
                this->hide();
            }
            else if (0 == strcmp(pdu->caData,LOGIN_FAILED))
            {
                QMessageBox::warning(this, "登录", "登录失败");
            }
            break;
        }
        case ENUM_MSG_TYPE_REVOKED_RESPOND://注销回复6
        {
            log << "注销回复";
            QMessageBox::information(this, "登录", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND://在线用户回复8
        {
            log << "在线用户回复";
            OpeWidget::getInstance().getFrined()->showAllOnlineUser(pdu);

            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_RESPOND://查找用户回复10
        {
            log << "查找用户回复" << pdu->caData;
            if(0 == strcmp(SEARCH_USR_NO,pdu->caData))
            {
                QMessageBox::information(this, "搜索用户", QString("%1用户不存在").arg(OpeWidget::getInstance().getFrined()->m_strSearchName));
                log << "用户不存在";
            }
            else if(0 == strcmp(SEARCH_USR_ONLINE,pdu->caData))
            {
                QMessageBox::information(this, "搜索用户", QString("%1用户在线").arg(OpeWidget::getInstance().getFrined()->m_strSearchName));
                log << "用户在线";
            }
            else if(0 == strcmp(SEARCH_USR_OFFLINE, pdu->caData))
            {
                QMessageBox::information(this, "搜索用户", QString("%1用户不在线").arg(OpeWidget::getInstance().getFrined()->m_strSearchName));
                log << "用户不在线";
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST://添加好友请求11
        {
            log << "添加好友请求";
            char caName[32] = {'\0'};
            strcpy(caName,pdu->caData + 32);
            int ret = QMessageBox::information(this, "添加好友",QString("%1希望与你成为好友,是否同意？").arg(caName),QMessageBox::Yes,QMessageBox::No);
            PDU *respdu = mkPDU(0);
            memcpy(respdu->caData,pdu->caData,64);//需要自己的用户名和对方的用户名
            if(ret == QMessageBox::Yes)
            {
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGGREE;//同意11
            }
            else
            {
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;//拒绝12
            }
            log << respdu->uiMsgType << " " <<respdu->caData;
            m_tcpSocket.write((char*)respdu,respdu->uiPDULen);//发送
            free(respdu);//释放
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND://添加好友回复12
        {
            log << "添加好友回复" << pdu->caData << (char*)pdu->caMsg;
            QMessageBox::information(this, "添加好友",QString("%1%2").arg((char*)pdu->caMsg).arg(pdu->caData));
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND://刷新好友回复16
        {
            log << "刷新好友回复";
            OpeWidget::getInstance().getFrined()->updateFriendList(pdu);
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST://删除好友请求17
        {
            log << "删除好友请求";
            char caName[32] ={'\0'};
            memcpy(caName,pdu->caData,32);
            QMessageBox::information(this,"删除好友",QString("%1删除你作为他的好友").arg(caName));
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND://删除好友回复18
        {
            log <<"删除好友回复";
            QMessageBox::information(this,"删除好友",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST://私聊请求19
        {
            log << "私聊请求";
            if(PrivateChat::getInstance().isHidden())
            {
                PrivateChat::getInstance().show();
            }
            char caSendName[32] = {'\0'};
            memcpy(caSendName, pdu->caData, 32);
            QString strSendName = caSendName;
            PrivateChat::getInstance().setChatName(strSendName);
            PrivateChat::getInstance().updateMsg(pdu);
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST://群聊请求21
        {
            log << "群聊请求";
            OpeWidget::getInstance().getFrined()->updateGroupChat(pdu);
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_RESPOND://创建文件回复24
        {
            log << "创建文件回复";
            QMessageBox::information(this, "创建文件夹", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND://刷新文件夹回复26
        {
            log << "刷新文件夹回复";
            OpeWidget::getInstance().getBook()->updateFileList(pdu);
            QString strEnterDir = OpeWidget::getInstance().getBook()->getEnterDir();
            log << m_strCurPath;
            if (!strEnterDir.isEmpty())
            {
                m_strCurPath = m_strCurPath+"/"+strEnterDir;
                log << m_strCurPath;
            }
            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_RESPOND://删除目录回复28
        {
            log << "删除目录回复";
            QMessageBox::information(this, "删除文件夹", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_RESPOND://重命名回复30
        {
            log << "重命名回复" << pdu->caData;
            QMessageBox::information(this, "重命名文件", pdu->caData);

            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_RESPOND://进入文件夹回复32
        {
            log << "进入文件夹回复";//不需要直接调用刷新
            OpeWidget::getInstance().getBook()->clrarEnterDir();
            QMessageBox::information(this, "进入文件夹", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND://上传文件回复34
        {
            log << "上传文件回复";
            QMessageBox::information(this, "上传文件", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND://下载文件回复36
        {
            log << "下载文件回复" << pdu->caData;
            char caFileName[32] = {'\0'};
            sscanf(pdu->caData, "%s %lld", caFileName, &(OpeWidget::getInstance().getBook()->m_iTotal));
            if (strlen(caFileName) > 0 && OpeWidget::getInstance().getBook()->m_iTotal > 0)
            {OpeWidget::getInstance().getBook()->setDownloadStatus(true);//设置下载状态
                m_file.setFileName(OpeWidget::getInstance().getBook()->getSaveFilePath());
                if (!m_file.open(QIODevice::WriteOnly))
                {
                    QMessageBox::warning(this, "下载文件", "获得保存文件的路径失败");
                }
            }
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_RESPOND://共享文件回复38
        {
            log << "共享文件回复";
            QMessageBox::information(this, "共享文件", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE://分享文件通知40
        {
            log << "分享文件通知";
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            char *pos = strrchr(pPath, '/');//c语言 重最后找字符
            if (NULL != pos)
            {
                pos++;
                QString strNote = QString("%1 分享文件->%2 \n 你是否接收?").arg(pdu->caData).arg(pos);
                int ret = QMessageBox::question(this, "共享文件", strNote);
                if (ret == QMessageBox::Yes)
                {
                    PDU *respdu = mkPDU(pdu->uiMsgLen);
                    respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
                    memcpy(respdu->caMsg, pdu->caMsg, pdu->uiMsgLen);
                    QString strName = TcpClient::getInstance().loginName();
                    strcpy(respdu->caData, strName.toStdString().c_str());
                    m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
                }
            }
            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_RESPOND://移动文件回复42
        {
            log << "移动文件回复";
            QMessageBox::information(this, "移动文件/文件夹", pdu->caData);
            break;
        }
        default:
            break;

        }
        free(pdu);//释放
        pdu = NULL;
    }
    else
    {
        QByteArray buffer = m_tcpSocket.readAll();
        m_file.write(buffer);
        Book *pBook = OpeWidget::getInstance().getBook();
        pBook->m_iRecved += buffer.size();
        if (pBook->m_iTotal == pBook->m_iRecved)
        {
            m_file.close();
            pBook->m_iTotal = 0;
            pBook->m_iRecved = 0;
            pBook->setDownloadStatus(false);
            QMessageBox::information(this, "下载文件", "下载文件成功");
        }
        else if (pBook->m_iTotal < pBook->m_iRecved)
        {
            m_file.close();
            pBook->m_iTotal = 0;
            pBook->m_iRecved = 0;
            pBook->setDownloadStatus(false);
            QMessageBox::critical(this, "下载文件", "下载文件失败");
        }
    }
}
//登录
void TcpClient::on_pushButton_login_clicked()
{
    QString strname = ui->lineEdit_name->text();
    QString strpwd = ui->lineEdit_pwd->text();
    if (strname.isEmpty() && strpwd.isEmpty())
    {
        QMessageBox::critical(this, "登录", "登录失败：用户名或密码为空");//弹窗警告

    }
    else
    {
        m_strLoginName = strname;
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;//注册请求
        memcpy(pdu->caData,strname.toStdString().c_str(),32);
        memcpy(pdu->caData + 32,strpwd.toStdString().c_str(),32);
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);//发送
        free(pdu);//释放
        pdu = NULL;
    }

}

//注册
void TcpClient::on_pushButton_regist_clicked()
{
    QString strname = ui->lineEdit_name->text();
    QString strpwd = ui->lineEdit_pwd->text();
    if (!strname.isEmpty() && !strpwd.isEmpty())
    {
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;//注册请求
        memcpy(pdu->caData,strname.toStdString().c_str(),32);
        memcpy(pdu->caData + 32,strpwd.toStdString().c_str(),32);
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);//发送
        free(pdu);//释放
        pdu = NULL;
    }
    else
    {
        QMessageBox::critical(this, "注册", "注册失败：用户名或密码不能为空");//弹窗警告
    }
}

//注销
void TcpClient::on_pushButton_revoked_clicked()
{
    QString strname = ui->lineEdit_name->text();
    QString strpwd = ui->lineEdit_pwd->text();
    if (!strname.isEmpty() && !strpwd.isEmpty())
    {
        int ret = QMessageBox::information(this, "注销",QString("注销后，您的信息将不可恢复，是否继续？"), QMessageBox::Yes, QMessageBox::No);
        if(ret == QMessageBox::Yes)
        {
            PDU *pdu = mkPDU(0);
            pdu->uiMsgType = ENUM_MSG_TYPE_REVOKED_REQUEST;//注销请求;
            memcpy(pdu->caData,strname.toStdString().c_str(),32);
            memcpy(pdu->caData + 32,strpwd.toStdString().c_str(),32);
            m_tcpSocket.write((char*)pdu,pdu->uiPDULen);//发送
            free(pdu);//释放
            pdu = NULL;
        }
    }
    else
    {
        QMessageBox::critical(this, "注销", "注销失败：用户名或密码不能为空");//弹窗警告
    }
}
