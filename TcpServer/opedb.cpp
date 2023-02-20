#include "opedb.h"
#include "utils.h"
#include <QMessageBox>

OpeDB::OpeDB(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
}

OpeDB &OpeDB::getInstance()
{
    static OpeDB instance;
    return instance;
}

void OpeDB::init()//载入数据库
{
    m_db.setHostName("localhost");//本地地址
    //m_db.setDatabaseName(":cloud.db");相对路径失败
    m_db.setDatabaseName("G:\\NetworkDisk\\TcpServer\\cloud.db");//使用绝对路径
    if(m_db.open())
    {
        log;
        QSqlQuery query;//执行数据库语句 使用QSqlQuery
        QString sql = "select * from usrInfo;";
        query.exec(sql);
        while(query.next())
        {
            QString data = QString("%1,%2,%3")
                    .arg(query.value(0).toString())
                    .arg(query.value(1).toString())
                    .arg(query.value(2).toString());
            log << data;
        }
    }
    else
    {
        QMessageBox::critical(NULL, "打开数据库", "打开数据库失败");//弹窗
    }
}

OpeDB::~OpeDB()
{
    m_db.close();
}

bool OpeDB::handleRegist(const char *name,const char *pwd)//注册
{
    if(NULL == name || NULL == pwd)
    {
        log << "NULL == name || NULL == pwd";
        return  false;
    }
    QSqlQuery query;//执行数据库语句 使用QSqlQuery
    QString sql = QString("insert into usrInfo(name,pwd) values(\'%1\',\'%2\')").arg(name).arg(pwd);
    log << sql;
    return query.exec(sql);
}

bool OpeDB::handleLogin(const char *name, const char *pwd)//登录
{
    if(NULL == name || NULL == pwd)
    {
        log << "NULL == name || NULL == pwd";
        return  false;
    }
    QSqlQuery query;//执行数据库语句 使用QSqlQuery
    QString sql = QString("select * from usrInfo where name = \'%1\' and pwd = \'%2\' and online = 0").arg(name).arg(pwd);
    log << sql;
    query.exec(sql);
    if (query.next())
    {
        sql = QString("update usrInfo set online = 1 where name = \'%1\' and pwd=\'%2\'").arg(name).arg(pwd);
        log << sql;
        query.exec(sql);
        return true;
    }
    else
    {
        log;
        return false;
    }
}

bool OpeDB::handleRevoked(const char *name, const char *pwd)//注销
{
    if(NULL == name || NULL == pwd)
    {
        log << "NULL == name || NULL == pwd";
        return false;
    }
    QSqlQuery query;//执行数据库语句 使用QSqlQuery
    QString sql = QString("select * from usrInfo where name = \'%1\' and pwd = \'%2\'").arg(name).arg(pwd);
    log << sql;
    query.exec(sql);
    if (query.next())
    {
        sql = QString("delete from friendInfo where id = (select id from usrInfo where name = \'%1\')").arg(name);
        log << sql;
        query.exec(sql);
        sql = QString("delete from friendInfo where friendId = (select id from usrInfo where name = \'%2\')").arg(name);
        log << sql;
        query.exec(sql);
        sql = QString("delete from usrInfo where name = \'%1\'").arg(name);
        log << sql;
        query.exec(sql);
        return true;
    }
    return false;
}

void OpeDB::handleOffline(const char *name)//处理下线
{
    if(NULL == name)
    {
        log << "NULL == name";
        return;
    }
    QSqlQuery query;//执行数据库语句 使用QSqlQuery
    QString sql = QString("update usrInfo set online = 0 where name = \'%1\'").arg(name);
    log << sql;
    query.exec(sql);
}

void OpeDB::serverOffline()//全部用户下线
{
    QSqlQuery query;//执行数据库语句 使用QSqlQuery
    QString sql = QString("update usrInfo set online = 0");
    log << sql;
    query.exec(sql);
}

QStringList OpeDB::handleAllOnline()//显示所有在线用户
{
    QSqlQuery query;//执行数据库语句 使用QSqlQuery
    QString sql = QString("select name from usrInfo where online = 1");
    log << sql;
    query.exec(sql);
    QStringList result;
    result.clear();
    while(query.next())
    {
        result.append(query.value(0).toString());
        log << query.value(0);
    }
    return result;
}

int OpeDB::handleSEarchUser(const char *name)//查询用户情况
{
    if(name == NULL)
    {
        return -1;//无用户名
    }
    QSqlQuery query;//执行数据库语句 使用QSqlQuery
    QString sql = QString("select online from usrInfo where name = \'%1\'").arg(name);
    log << sql;
    query.exec(sql);
    if (query.next())
    {
        int ret = query.value(0).toInt();
        if(ret == 1)
        {
            return 1;
        }
        else if(ret == 0)
        {
            return 0;
        }
    }
    return -1;//无用户
}

int OpeDB::handleAddFriend(const char *pername, const char *name)//添加好友判断对方状态
{
    if(NULL == name || NULL == pername)
    {
        log << "NULL == name || NULL == pername";
        return  -1;
    }
    QSqlQuery query;//执行数据库语句 使用QSqlQuery
    QString sql =  QString("select * from friend where (id = (select id from usrInfo where name = \'%1\') and friendId = (select id from usrInfo where name = \'%2\')) "
                           "or (id = (select id from usrInfo where name = \'%3\') and friendId = (select id from usrInfo where name = \'%4\'))").arg(pername).arg(name).arg(name).arg(pername);
    log << sql;
    query.exec(sql);
    if (query.next())//判断是否为好友
    {
        log << "双方已经是好友";
        return 0;//双方已经是好友
    }
    else
    {
        sql = QString("select online from usrInfo where name = \'%1\'").arg(name);
        log << sql;
        query.exec(sql);
        if (query.next())
        {
            log <<query.value(0);
            int ret = query.value(0).toInt();
            if(ret == 1)
            {
            log << "在线";
            return 1;//在线
            }
            else if(ret == 0)
            {
            log << "离线";
            return 2;//离线
            }
        }
        else
        {
              log << "无用户";
            return 3;//无用户
        }
    }
}

void OpeDB::handleAgrAddFriend(const char *pername, const char *name)//同意添加好友
{
    if(NULL == name || NULL == pername)
    {
        log << "NULL == name || NULL == pername";
        return;
    }
    QSqlQuery query;//执行数据库语句 使用QSqlQuery
    QString sql = QString("insert into friendInfo(id, friendId) values((select id from usrInfo where name=\'%1\'), (select id from usrInfo where name=\'%2\'))")
            .arg(pername).arg(name);
    log << sql;
    query.exec(sql);
}

QStringList OpeDB::handleFlushFriend(const char *name)//刷新好友列表
{
    QStringList strFriendList;
    strFriendList.clear();
    if(name == NULL)
    {
        return strFriendList;
    }
    QSqlQuery query;//执行数据库语句 使用QSqlQuery
    //查自己的为friendid
    QString sql = QString("select name from usrInfo where online = 1 and id in (select id from friendInfo where friendId = (select id from usrInfo where name=\'%1\'))").arg(name);
    log << sql;
    query.exec(sql);
    while(query.next())
    {
        strFriendList.append(query.value(0).toString());
        log << query.value(0);
    }
    //查自己为id
    sql = QString("select name from usrInfo where online = 1 and id in (select friendId from friendInfo where id = (select id from usrInfo where name=\'%1\'))").arg(name);
    log << sql;
    query.exec(sql);
    while(query.next())
    {
        strFriendList.append(query.value(0).toString());
        log << query.value(0);
    }
    return strFriendList;
}

bool OpeDB::handleDeleteFriend(const char *pername, const char *name)//删除好友
{
    if(NULL == name || NULL == pername)
    {
        log << "NULL == name || NULL == pername";
        return  false;
    }
    QSqlQuery query;//执行数据库语句 使用QSqlQuery
    QString sql = QString("delete from friendInfo where id = (select id from usrInfo where name = \'%1\') and friendId = (select id from usrInfo where name=\'%2\')").arg(name).arg(pername);
    log << sql;
    query.exec(sql);
    sql = QString("delete from friendInfo where id = (select id from usrInfo where name = \'%1\') and friendId = (select id from usrInfo where name=\'%2\')").arg(pername).arg(name);
    log << sql;
    query.exec(sql);
    return true;
}
