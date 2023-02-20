#ifndef OPEWIDGET_H
#define OPEWIDGET_H

//客户端登录成功界面

#include <QWidget>
#include <QListWidget>
#include "friend.h"
#include "book.h"
#include <QStackedWidget>//堆栈窗口

class OpeWidget : public QWidget
{
    Q_OBJECT
private:
    explicit OpeWidget(QWidget *parent = nullptr);
public:
    static OpeWidget &getInstance();
    Friend *getFrined();
    Book *getBook();

signals:



private:
    QListWidget *m_plistW;
    Friend *m_pFriend;
    Book *m_pBook;


    QStackedWidget * m_pSW;
};

#endif // OPEWIDGET_H
