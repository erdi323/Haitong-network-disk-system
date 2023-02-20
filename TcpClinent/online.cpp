#include "online.h"
#include "ui_online.h"
#include "tcpclient.h"

Online::Online(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

void Online::showUser(PDU *pdu)
{
    if(NULL == pdu)
    {
        return;
    }
    ui->listWidget_onlie->clear();
    uint uiSize = pdu->uiMsgLen/32;
    char caTmp[32];
    for (uint i = 0; i < uiSize; i++)
    {
        memcpy(caTmp,(char*)(pdu->caMsg) + i * 32, 32);
        log << caTmp;
        if(caTmp == TcpClient::getInstance().loginName())//不显示自己
        {
            continue;
        }
        ui->listWidget_onlie->addItem(caTmp);
    }
}

void Online::on_pushButton_addfriend_clicked()
{
    QListWidgetItem *pItem = ui->listWidget_onlie->currentItem();
    log << pItem->text();
    QString strPerUserName = pItem->text();//需要添加的用户名
    QString strLoginName = TcpClient::getInstance().loginName();//自己的名字
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType =ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    strcpy(pdu->caData,strPerUserName.toStdString().c_str());
    strcpy(pdu->caData + 32,strLoginName.toStdString().c_str());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//发送
    free(pdu);//释放
    pdu = NULL;
}

