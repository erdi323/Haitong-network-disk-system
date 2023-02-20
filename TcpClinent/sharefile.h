#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QScrollArea>
#include <QCheckBox>
#include <QListWidget>

class ShareFile : public QWidget
{
    Q_OBJECT  
public:
    explicit ShareFile(QWidget *parent = nullptr);
    static ShareFile &getInstance();

    void test();




    void updateFriend(QListWidget *pFriendList);

signals:

public slots:
    void cancelSelect();//取消选择
    void selectAll();//全选

    void okShare();//确认
    void cancelShare();//取消

private:


    QPushButton *m_pSelectAllPB;//全选
    QPushButton *m_pCancelSelectPB;//反选

    QPushButton *m_pOKPB;//确认分享
    QPushButton *m_pCancelPB;//取消分享

    QScrollArea *m_pSA;//好友展示区
    QWidget *m_pFriendW;//好友列表
    QVBoxLayout *m_pFriendWVBL;//垂直布局
    QButtonGroup *m_pButtonGroup;//管理好友

};

#endif // SHAREFILE_H
