#include "privatechat.h"
#include "ui_privatechat.h"
#include <QMessageBox>
#include "tcpclient.h"

PrivateChat::PrivateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

PrivateChat &PrivateChat::getInstance()
{
    static PrivateChat instance;
    return instance;
}

void PrivateChat::setChatName(QString strName)//修改聊天名称
{
    m_strChatName = strName;
    m_strLoginName = TcpClient::getInstance().loginName();
}

void PrivateChat::updateMsg(const PDU *pdu)//更新聊天信息
{
    if(pdu == NULL)
    {
        return;
    }
    char caSendName[32] = {'\0'};
    memcpy(caSendName,pdu->caData,32);
    QString strMsg = QString("%1 : %2").arg(caSendName).arg((char*)pdu->caMsg);
    ui->textEdit_showMsg_te->append(strMsg);
}

void PrivateChat::on_pushButton_sendMsg_te_clicked()
{
    QString strMsg = ui->lineEdit_inputMsg_le->text();
    ui->lineEdit_inputMsg_le->clear();
    if(!strMsg.isEmpty())
    {
        PDU *pdu = mkPDU(strMsg.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
        memcpy(pdu->caData, m_strLoginName.toStdString().c_str(), m_strLoginName.size());
        memcpy(pdu->caData+32, m_strChatName.toStdString().c_str(), m_strChatName.size());
        strcpy((char*)(pdu->caMsg), strMsg.toStdString().c_str());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        ui->textEdit_showMsg_te->append(QString("%1 : %2").arg(m_strLoginName).arg(strMsg));
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::warning(this,"私聊","发送内容为空");
    }

}
