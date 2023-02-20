#ifndef PROTOCOL_H
#define PROTOCOL_H

//协议

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef unsigned int uint; //将无符号整型缩写成uint

#define REGIST_OK "注册成功"
#define REGIST_FAILED "注册失败，用户名重复"

#define LOGIN_OK "登录成功"
#define LOGIN_FAILED "登录失败:用户名或密码错误或重复登录"

#define SEARCH_USR_NO "查无用户"
#define SEARCH_USR_ONLINE "该用户在线"
#define SEARCH_USR_OFFLINE "该用户不在线"

#define UNKNOW_ERROR "未知错误"
#define EXISTED_FRIEND "好友也存在"
#define ADD_FRIEND_OFFLINE "用户不在线"
#define ADD_FRIEND_NOEXIT "用户不存在"

#define ADD_FRIEND_AGGREE "同意添加你为好友"
#define ADD_FRIEND_REFUSE "拒绝添加你为好友"

#define ADD_FRIEND_OK "添加好友成功"
#define ADD_FRIEND_NO "添加好友失败"

#define DEL_FRIEND_OK "删除好友成功"

#define DIR_NO_EXIST "路径不存在"
#define FILE_NAME_EXIST "文件名已存在"
#define CREAT_DIR_OK "创建文件夹成功"

#define DEL_DIR_OK "删除成功"
#define DEL_DIR_FAILED "删除失败"

#define RENAME_OK "重命名成功"
#define RENAME_FAILED "重命名失败"

#define ENTER_DIR_FAILURED "进入文件夹失败"

#define UPLOAD_FILE_OK "上传文件成功"
#define UPLOAD_FILE_FAILURED "上传文件失败"

#define MOVE_FILE_OK "移动文件成功"
#define MOVE_FILE_FAILURED "移动文件失败,移动路径不是文件夹"
#define COMMON_ERR "系统繁忙，操作失败"

enum ENUM_MSG_TYPE//消息类型
{
    ENUM_MSG_TYPE_MIN = 0,
    ENUM_MSG_TYPE_REGIST_REQUEST,       //注册请求 1
    ENUM_MSG_TYPE_REGIST_RESPOND,       //注册回复 2

    ENUM_MSG_TYPE_LOGIN_REQUEST,        //登录请求 3
    ENUM_MSG_TYPE_LOGIN_RESPOND,        //登录回复 4

    ENUM_MSG_TYPE_REVOKED_REQUEST,        //注销请求 5
    ENUM_MSG_TYPE_REVOKED_RESPOND,        //注销回复 6

    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,   //在线用户请求 7
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND,   //在线用户回复 8

    ENUM_MSG_TYPE_SEARCH_USR_REQUEST,   //搜索用户请求 9
    ENUM_MSG_TYPE_SEARCH_USR_RESPOND,   //搜索用户回复 10

    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST,   //添加好友请求 11
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND,   //添加好友回复 12

    ENUM_MSG_TYPE_ADD_FRIEND_AGGREE,    //同意添加好友 13
    ENUM_MSG_TYPE_ADD_FRIEND_REFUSE,    //拒绝添加好友 14

    ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST, //刷新好友请求 15
    ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND, //刷新好友回复 16

    ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST,//删除好友请求 17
    ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND,//删除好友回复 18

    ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST, //私聊请求 19
    ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND, //私聊回复 20

    ENUM_MSG_TYPE_GROUP_CHAT_REQUEST,   //群聊请求 21
    ENUM_MSG_TYPE_GROUP_CHAT_RESPOND,   //群聊回复 22

    ENUM_MSG_TYPE_CREATE_DIR_REQUEST,   //创建文件夹请求 23
    ENUM_MSG_TYPE_CREATE_DIR_RESPOND,   //创建文件夹回复 24

    ENUM_MSG_TYPE_FLUSH_FILE_REQUEST,   //刷新文件请求 25
    ENUM_MSG_TYPE_FLUSH_FILE_RESPOND,   //刷新文件回复 26

    ENUM_MSG_TYPE_DEL_DIR_REQUEST,      //删除文件/文件夹请求 27
    ENUM_MSG_TYPE_DEL_DIR_RESPOND,      //删除文件/文件夹回复 28

    ENUM_MSG_TYPE_RENAME_FILE_REQUEST,  //重命名文件请求 29
    ENUM_MSG_TYPE_RENAME_FILE_RESPOND,  //重命名文件回复 30

    ENUM_MSG_TYPE_ENTER_DIR_REQUEST,    //进入文件夹请求 31
    ENUM_MSG_TYPE_ENTER_DIR_RESPOND,    //进入文件夹回复 32

    ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST,  //上传文件请求 33
    ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND,  //上传文件回复 34


    ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST,//下载文件请求 35
    ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND,//下载文件回复 36

    ENUM_MSG_TYPE_SHARE_FILE_REQUEST,   //共享文件请求 37
    ENUM_MSG_TYPE_SHARE_FILE_RESPOND,   //共享文件回复 38

    ENUM_MSG_TYPE_SHARE_FILE_NOTE,      //分享文件通知 39
    ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND,//分享文件通知回复 40

    ENUM_MSG_TYPE_MOVE_FILE_REQUEST,    //移动文件请求 41
    ENUM_MSG_TYPE_MOVE_FILE_RESPOND,    //移动文件回复 42

    ENUM_MSG_TYPE_MAX = 0x00ffffff
};

struct PDU  //协议数据单元protocol data unity
{
    uint uiPDULen;  //总的协议数据单元大小
    uint uiMsgType; //消息类型
    char caData[64];//文件名
    uint uiMsgLen;  //实际消息长度
    int caMsg[];    //实际消息
};

PDU * mkPDU(uint uiMsgLen); //构造函数



struct FileInfo
{
    char caFileName[32];//文件名
    int iFileType;//文件类型
};



#endif // PROTOCOL_H
