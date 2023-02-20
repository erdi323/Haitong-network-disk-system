#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "online.h"
#include "protocol.h"

class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);

    QString m_strSearchName;

    void showAllOnlineUser(PDU *pdu);//显示小部件

    void updateFriendList(PDU *pdu);//刷新好友列表

    void updateGroupChat(PDU *pdu);//刷新群聊消息列表

    QListWidget *getFriendList();

signals:

public slots:
    void showOnline();//隐藏小部件
    void searchUser();//查找用户
    void flushFriend();//刷新好友列表
    void deteleFriend();//删除好友
    void privateChat();//私聊
    void groupChat();//群聊

private:
    QTextEdit *m_pShowMsgTE;//显示信息
    QListWidget *m_pFriendListWidget;//好友列表
    QLineEdit *m_pInputMsgLe;//输入消息框

    QPushButton *m_pDelFriendPB;//删除好友
    QPushButton *m_pFlushFriendPB;//刷新在线好友列表
    QPushButton *m_pShowOnlineUserPB;//查看在线用户
    QPushButton *m_pSearchUsrPB ;//查找用户
    QPushButton *m_pMsgSendPB;//发送消息
    QPushButton *m_pPrivateChatPB;//私聊

    Online *m_pOnline;
};

#endif // FRIEND_H
