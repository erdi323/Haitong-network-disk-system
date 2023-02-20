#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include <QSqlDatabase>//连接数据库使用
#include <QSqlQuery>//查询数据库使用
#include <QStringList>

class OpeDB : public QObject
{
    Q_OBJECT
private:
    explicit OpeDB(QObject *parent = nullptr);
    QSqlDatabase m_db;//连接数据库

public:
    static OpeDB& getInstance();

    void init();//载入数据库

    ~OpeDB();

    bool handleRegist(const char *name, const char *pwd);//注册

    bool handleLogin(const char *name, const char *pwd);//登录

    bool handleRevoked(const char *name, const char *pwd);//注销

    void handleOffline(const char *name);//全体用户下线

    void serverOffline();//全部用户下线

    QStringList handleAllOnline();//在线用户

    int handleSEarchUser(const char *name);//查询用户情况

    int handleAddFriend(const char *pername, const char *name);//判断是否为好友

    void handleAgrAddFriend(const char *pername, const char *name);//同意添加好友

    QStringList handleFlushFriend(const char *name);//刷新好友

    bool handleDeleteFriend(const char *pername, const char *name);//删除好友
signals:


public slots:
};

#endif // OPEDB_H
