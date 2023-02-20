#include "friend.h"
#include "tcpclient.h"
#include <QInputDialog>//输入数据
#include "privatechat.h"
#include <QMessageBox>
Friend::Friend(QWidget *parent) : QWidget(parent)
{
    m_pShowMsgTE = new QTextEdit;//显示信息
    m_pFriendListWidget = new QListWidget;//好友列表
    m_pInputMsgLe = new QLineEdit;//输入消息框

    m_pDelFriendPB = new QPushButton("删除好友");//删除好友
    m_pFlushFriendPB = new QPushButton("刷新好友列表");//刷新在线好友列表
    m_pShowOnlineUserPB = new QPushButton("显示在线用户");//查看在线用户
    m_pSearchUsrPB = new QPushButton("查找用户");//查找用户
    m_pMsgSendPB = new QPushButton("发送");//发送消息
    m_pPrivateChatPB = new QPushButton("私聊");//私聊

    QVBoxLayout *pRightPBVBL = new QVBoxLayout;//垂直布局
    pRightPBVBL->addWidget(m_pDelFriendPB);
    pRightPBVBL->addWidget(m_pFlushFriendPB);
    pRightPBVBL->addWidget(m_pShowOnlineUserPB);
    pRightPBVBL->addWidget(m_pSearchUsrPB);
    pRightPBVBL->addWidget(m_pPrivateChatPB);

    QHBoxLayout *pTopHBL = new QHBoxLayout;//水平布局
    pTopHBL->addWidget(m_pShowMsgTE);
    pTopHBL->addWidget(m_pFriendListWidget);
    pTopHBL->addLayout(pRightPBVBL);

    QHBoxLayout *pMsgHBL = new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLe);
    pMsgHBL->addWidget(m_pMsgSendPB);

    m_pOnline = new Online;

    QVBoxLayout *pMain = new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(m_pOnline);
    m_pOnline->hide();//隐藏
    setLayout(pMain);

    QObject::connect(m_pShowOnlineUserPB,&QPushButton::clicked,
                     this,&Friend::showOnline);
    QObject::connect(m_pSearchUsrPB,&QPushButton::clicked,
                     this,&Friend::searchUser);
    QObject::connect(m_pFlushFriendPB,&QPushButton::clicked,
                     this,&Friend::flushFriend);
    QObject::connect(m_pDelFriendPB,&QPushButton::clicked,
                     this,&Friend::deteleFriend);
    QObject::connect(m_pPrivateChatPB,&QPushButton::clicked,
                     this,&Friend::privateChat);
    QObject::connect(m_pMsgSendPB,&QPushButton::clicked,
                     this,&Friend::groupChat);
}


void Friend::showAllOnlineUser(PDU *pdu)//显示小部件
{
    if(NULL == pdu)
    {
        return;
    }
    m_pOnline->showUser(pdu);
}

void Friend::updateFriendList(PDU *pdu)//刷新好友列表
{
    m_pFriendListWidget->clear();
    if(NULL == pdu)
    {
        return;
    }
    uint uiSize = pdu->uiMsgLen / 32;
    char caName[32] = {'\0'};
    for (uint i =0; i < uiSize; i++)
    {
        memcpy(caName,(char*)pdu->caMsg + i *32,32);
        m_pFriendListWidget->addItem(caName);
    }
}

void Friend::updateGroupChat(PDU *pdu)
{
    m_pShowMsgTE->append(QString("%1: %2").arg(pdu->caData).arg((char*)pdu->caMsg));
}

QListWidget *Friend::getFriendList()
{
    return m_pFriendListWidget;
}

void Friend::showOnline()//隐藏小部件
{
    if(m_pOnline->isHidden())//小部件是否被隐藏
    {
        m_pOnline->show();
        PDU *pdu =mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;//在线用户请求
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//发送
        free(pdu);
        pdu = NULL;
    }
    else
    {
        m_pOnline->hide();
    }
}

void Friend::searchUser()//查找用户
{
    m_strSearchName = QInputDialog::getText(this,"搜索","用户名");
    if(!m_strSearchName.isEmpty())
    {
        log << m_strSearchName;
        PDU *pdu = mkPDU(0);
        strcpy(pdu->caData,m_strSearchName.toStdString().c_str());
        pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_REQUEST;
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//发送
        log << pdu->caData;
        free(pdu);
        pdu = NULL;
    }
}

void Friend::flushFriend()//刷新好友
{
    QString strName =TcpClient::getInstance().loginName();
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;//刷新好友请求
    strcpy(pdu->caData,strName.toStdString().c_str());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//发送
    free(pdu);//释放
    pdu = NULL;
}

void Friend::deteleFriend()//删除好友
{
    if(m_pFriendListWidget->currentItem() != NULL)
    {
        int ret = QMessageBox::information(this, "删除好友",QString("删除好友将不可恢复是否继续？"), QMessageBox::Yes, QMessageBox::No);
        if(ret == QMessageBox::Yes)
        {
            QString strFriendName = m_pFriendListWidget->currentItem()->text();
            QString strName = TcpClient::getInstance().loginName();
            log << strFriendName;
            PDU *pdu =mkPDU(0);
            memcpy(pdu->caData, strName.toStdString().c_str(), strName.size());
            memcpy(pdu->caData + 32, strFriendName.toStdString().c_str(), strFriendName.size());
            pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;//删除好友请求
            TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//发送
            free(pdu);
            pdu = NULL;
        }
    }
}

void Friend::privateChat()//私聊
{
    if(m_pFriendListWidget->currentItem() != NULL)
    {
        QString strName = m_pFriendListWidget->currentItem()->text();//获得好友名字
        PrivateChat::getInstance().setChatName(strName);
        if(PrivateChat::getInstance().isHidden())
        {
            PrivateChat::getInstance().show();
        }
    }
    else
    {
        QMessageBox::warning(this,"私聊","请选择私聊对象");
    }
}

void Friend::groupChat()//群聊
{
    QString strMsg = m_pInputMsgLe->text();
    m_pInputMsgLe->clear();
    if(!strMsg.isEmpty())
    {
        PDU *pdu = mkPDU(strMsg.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
        QString strName = TcpClient::getInstance().loginName();
        memcpy(pdu->caData, strName.toStdString().c_str(), strName.size());
        strcpy((char*)(pdu->caMsg), strMsg.toStdString().c_str());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        m_pShowMsgTE->append(QString("%1: %2").arg(strName).arg(strMsg));
    }
    else
    {
        QMessageBox::warning(this,"群聊","发送内容为空");
    }
}
