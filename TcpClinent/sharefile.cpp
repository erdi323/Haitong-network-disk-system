#include "sharefile.h"
#include "protocol.h"
#include "tcpclient.h"
#include "opewidget.h"

ShareFile::ShareFile(QWidget *parent) : QWidget(parent)
{
    m_pSelectAllPB = new QPushButton("全选");
    m_pCancelSelectPB = new QPushButton("取消选择");

    m_pOKPB = new QPushButton("确定");
    m_pCancelPB = new QPushButton("取消");

    m_pSA = new QScrollArea;
    m_pFriendW = new QWidget;

    m_pFriendWVBL = new QVBoxLayout(m_pFriendW);
    m_pButtonGroup = new QButtonGroup(m_pFriendW);
    m_pButtonGroup->setExclusive(false);//勾选框

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pSelectAllPB);
    pTopHBL->addWidget(m_pCancelSelectPB);
    pTopHBL->addStretch();//弹簧

    QHBoxLayout *pDownHBL = new QHBoxLayout;
    pDownHBL->addWidget(m_pOKPB);
    pDownHBL->addWidget(m_pCancelPB);

    QVBoxLayout *pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);
    pMainVBL->addWidget(m_pSA);
    pMainVBL->addLayout(pDownHBL);

    //test();

    setLayout(pMainVBL);

    connect(m_pCancelSelectPB, &QPushButton::clicked,
             this, &ShareFile::cancelSelect);

     connect(m_pSelectAllPB, &QPushButton::clicked,
             this, &ShareFile::selectAll);

     connect(m_pOKPB, &QPushButton::clicked,
             this, &ShareFile::okShare);

     connect(m_pCancelPB, &QPushButton::clicked,
             this, &ShareFile::cancelShare);

}

void ShareFile::test()//测试
{
    //QVBoxLayout *p = new QVBoxLayout(m_pFriendW);布局冲突
    QCheckBox *pCB = NULL;
    for (int i = 0; i < 15; i++)
    {
        log << i;
        pCB = new QCheckBox(QString("tom%1").arg(i));
        m_pFriendWVBL->addWidget(pCB);
        m_pButtonGroup->addButton(pCB);
    }
    m_pSA->setWidget(m_pFriendW);
}

ShareFile &ShareFile::getInstance()
{
    static ShareFile instance;
    return instance;
}
void ShareFile::updateFriend(QListWidget *pFriendList)//刷新分享文件中的好友列表
{
    if (NULL == pFriendList)
    {
        return;
    }
    QAbstractButton *tmp = NULL;
    QList<QAbstractButton*> preFriendList = m_pButtonGroup->buttons();//之前列表的信息
    for (int i = 0; i < preFriendList.size(); i++)//移除之前的信息
    {
        tmp = preFriendList[i];
        m_pFriendWVBL->removeWidget(tmp);
        m_pButtonGroup->removeButton(tmp);
        preFriendList.removeOne(tmp);
        delete tmp;
        tmp = NULL;
    }
    QCheckBox *pCB = NULL;
    for (int i = 0; i < pFriendList->count(); i++)//添加
    {
        pCB = new QCheckBox(pFriendList->item(i)->text());
        m_pFriendWVBL->addWidget(pCB);
        m_pButtonGroup->addButton(pCB);
    }
    m_pSA->setWidget(m_pFriendW);
}

void ShareFile::cancelSelect()//取消选择
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for (int i=0; i<cbList.size(); i++)
    {
        if (cbList[i]->isChecked())
        {
            cbList[i]->setChecked(false);
        }
    }
}

void ShareFile::selectAll()//选择全部
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for (int i=0; i<cbList.size(); i++)
    {
        if (!cbList[i]->isChecked())
        {
            cbList[i]->setChecked(true);
        }
    }
}

void ShareFile::okShare()//确认
{
    QString strName = TcpClient::getInstance().loginName();//用户
    QString strCurPath = TcpClient::getInstance().curPath();//用户的文件夹
    QString strShareFileName = OpeWidget::getInstance().getBook()->getShareFileName();//分享的文件名

    QString strPath = strCurPath+"/"+strShareFileName;//文件总路径

    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();//获得好友列表信息
    int num = 0;//好友个数
    for (int i=0; i<cbList.size(); i++)
    {
        if (cbList[i]->isChecked())
        {
            num++;
        }
    }

    PDU *pdu = mkPDU(32*num+strPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    sprintf(pdu->caData, "%s %d", strName.toStdString().c_str(), num);
    int j = 0;
    for (int i=0; i<cbList.size(); i++)
    {
        if (cbList[i]->isChecked())
        {
            memcpy((char*)(pdu->caMsg)+j*32, cbList[i]->text().toStdString().c_str(), cbList[i]->text().size());
            j++;
        }
    }

    memcpy((char*)(pdu->caMsg)+num*32, strPath.toStdString().c_str(), strPath.size());

    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);

    free(pdu);
    pdu = NULL;
    hide();
}

void ShareFile::cancelShare()//取消
{
    hide();
}

