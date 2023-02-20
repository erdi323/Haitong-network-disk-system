#include "opewidget.h"
#include "tcpclient.h"
OpeWidget::OpeWidget(QWidget *parent) : QWidget(parent)
{
    m_plistW = new QListWidget(this);
    m_plistW->addItem("好友");
    m_plistW->addItem("图书");

    m_pFriend = new Friend;
    m_pBook = new Book;

    m_pSW = new QStackedWidget;
    m_pSW->addWidget(m_pFriend);
    m_pSW->addWidget(m_pBook);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_plistW);
    pMain->addWidget(m_pSW);
    setLayout(pMain);

    this->setWindowTitle(TcpClient::getInstance().loginName());//修改窗口名为用户名

    QObject::connect(m_plistW,&QListWidget::currentRowChanged,
                     m_pSW,&QStackedWidget::setCurrentIndex);
}

OpeWidget &OpeWidget::getInstance()
{
    static OpeWidget instance;
    return instance;
}

Friend *OpeWidget::getFrined()
{
    return m_pFriend;
}

Book *OpeWidget::getBook()
{
    return m_pBook;
}
