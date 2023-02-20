#include "mytcpsocket.h"
#include "utils.h"
#include "mytcpserver.h"
#include <QDir>
#include <QFileInfoList>

MyTcpSocket::MyTcpSocket()
{
    QObject::connect(this,SIGNAL(readyRead()),//&MyTcpSocket::readyRead,
                     this,SLOT(recvMsg()));//&MyTcpSocket::recvMsg);
    QObject::connect(this,SIGNAL(disconnected()), //&MyTcpSocket::disconnected,
                     this,SLOT(clientOffline())); //&MyTcpSocket::clientOffline);

    m_bUpload = false;
    m_pTimer = new QTimer;
    connect(m_pTimer, &QTimer::timeout,
            this, &MyTcpSocket::sendFileToClient);
}

void MyTcpSocket::recvMsg()//防止收发数据公用
{
    if(!m_bUpload)
    {
        log << this->bytesAvailable();
        uint uiPDULen = 0;
        this->read((char*)&uiPDULen, sizeof(uint));//获得信息总大小
        uint uiMsgLen = uiPDULen-sizeof (PDU);
        PDU *pdu = mkPDU(uiMsgLen);
        this->read((char*)pdu + sizeof (uint),uiPDULen - sizeof (uint));//读取剩余消息数据
        log << pdu->uiMsgType << " "<< (char*)pdu->caMsg;
        switch (pdu->uiMsgType)
        {
        case ENUM_MSG_TYPE_REGIST_REQUEST://注册请求1
        {
            log << "注册请求";
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            memcpy(caName, pdu->caData, 32);
            memcpy(caPwd, pdu->caData + 32, 32);
            bool ret = OpeDB::getInstance().handleRegist(caName,caPwd);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;//注册回复
            if(ret)
            {
                strcpy(respdu->caData,REGIST_OK);
                QDir dir;
                log << REGIST_OK << dir.mkdir(QString("./%1").arg(caName));//创建文件夹
            }
            else
            {
                strcpy(respdu->caData,REGIST_FAILED);
                log << REGIST_FAILED;
            }
            write((char*)respdu,respdu->uiPDULen);//发送
            free(respdu);//释放
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_REQUEST://登录请求3
        {
            log << "登录请求";
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            memcpy(caName, pdu->caData, 32);
            memcpy(caPwd, pdu->caData + 32, 32);
            bool ret = OpeDB::getInstance().handleLogin(caName,caPwd);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;//登录回复
            if(ret)
            {
                strcpy(respdu->caData,LOGIN_OK);
                m_strName = caName;
                log << LOGIN_OK;
                QDir dir;
                bool ret = dir.exists(QString("./%1").arg(caName));//判断是否存在
                if(!ret)
                {
                    dir.mkdir(QString("./%1").arg(caName));//创建文件夹
                }

            }
            else
            {
                strcpy(respdu->caData,LOGIN_FAILED);
                log << LOGIN_FAILED;
            }
            write((char*)respdu,respdu->uiPDULen);//发送
            free(respdu);//释放
            respdu = NULL;
            break;
            break;
        }
        case ENUM_MSG_TYPE_REVOKED_REQUEST://注销请求5
        {
            log << "注销请求";
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            memcpy(caName, pdu->caData, 32);
            memcpy(caPwd, pdu->caData + 32, 32);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_REVOKED_RESPOND;//注销回复
            bool ret = OpeDB::getInstance().handleRevoked(caName,caPwd);
            if(ret)
            {
                QDir dir;
                ret = dir.exists(QString("./%1").arg(caName));//判断是否存在
                if(ret)
                {
                    dir.setPath(QString("./%1").arg(caName));//设置到文件夹
                    dir.removeRecursively();//删除文件夹
                }
                strcpy(respdu->caData,"注销成功");
            }
            else
            {
                strcpy(respdu->caData,"注销失败，用户名或密码错误，或用户不存在");
            }
            write((char*)respdu,respdu->uiPDULen);//发送
            free(respdu);//释放
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST://在线用户请求请求7
        {
            log << "在线用户请求请求";
            QStringList ret = OpeDB::getInstance().handleAllOnline();
            uint uiMsgLen = ret.size() * 32;//每个名字占32字节
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;//登录回复
            for(int i = 0; i < ret.size(); i++)
            {
                memcpy((char*)(respdu->caMsg) + i * 32,
                       ret.at(i).toStdString().c_str(),
                       ret.at(i).size());
            }
            log << (char*)respdu->caMsg;
            write((char*)respdu,respdu->uiPDULen);//发送
            free(respdu);//释放
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_REQUEST://查找请求9
        {
            log << "查找请求";
            int ret = OpeDB::getInstance().handleSEarchUser(pdu->caData);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;//查找回复
            if(ret == -1)
            {
                strcpy(respdu->caData,SEARCH_USR_NO);//查无用户
            }
            else if (ret == 1)
            {
                strcpy(respdu->caData,SEARCH_USR_ONLINE);//用户在线
            }
            else if(ret == 0)
            {
                strcpy(respdu->caData,SEARCH_USR_OFFLINE);//用户不在线
            }
            log << respdu->caData;
            write((char*)respdu,respdu->uiPDULen);//发送
            free(respdu);//释放
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST://添加好友请求11
        {
            log << "添加好友请求";
            char caPername[32] = {'\0'};//对方
            char caName[32] = {'\0'};//自己
            memcpy(caPername, pdu->caData, 32);
            memcpy(caName, pdu->caData + 32, 32);
            int ret = OpeDB::getInstance().handleAddFriend(caPername,caName);
            PDU *respdu = NULL;
            log << ret;
            if (ret == -1)
            {
                log << "未知错误";
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;//添加好友回复
                strcpy(respdu->caData,UNKNOW_ERROR);//未知错误
                log << respdu->caData;
                write((char*)respdu,respdu->uiPDULen);//发送
            }
            else if (ret == 0)
            {
                log << "好友也存在";
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;//添加好友回复
                strcpy(respdu->caData,EXISTED_FRIEND);//好友也存在
                log << respdu->caData;
                write((char*)respdu,respdu->uiPDULen);//发送
            }
            else if (ret == 1)
            {
                log << "请求中";
                MyTcpServer::getInstance().resend(caPername,pdu);
            }
            else if (ret == 2)
            {
                log << "用户不在线";
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;//添加好友回复
                strcpy(respdu->caData,ADD_FRIEND_OFFLINE);//用户不在线
                log << respdu->caData;
                write((char*)respdu,respdu->uiPDULen);//发送
            }
            else if (ret == 3)
            {
                log << "用户不存在";
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;//添加好友回复
                strcpy(respdu->caData,ADD_FRIEND_NOEXIT);//用户不存在
                log << respdu->caData;
                write((char*)respdu,respdu->uiPDULen);//发送
            }
            free(respdu);//释放
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGGREE://同意添加好友13
        {
            log << "对方同意";
            char caPername[32] = {'\0'};
            char caName[32] = {'\0'};
            memcpy(caPername, pdu->caData, 32);//对方
            memcpy(caName,pdu->caData + 32, 32);
            PDU *respdu = mkPDU(32);
            strcpy(respdu->caData,ADD_FRIEND_AGGREE);
            memcpy((char*)respdu->caMsg,caName, 32);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;//添加好友回复
            OpeDB::getInstance().handleAgrAddFriend(caPername,caName);//数据库操作
            MyTcpServer::getInstance().resend(caPername,respdu);
            free(respdu);//释放
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE://拒绝添加好友14
        {
            log << "对方拒绝";
            char caPername[32] = {'\0'};
            memcpy(caPername, pdu->caData, 32);
            PDU *respdu = mkPDU(32);
            strcpy(respdu->caData,ADD_FRIEND_REFUSE);
            memcpy((char*)respdu->caMsg,pdu->caData+32, 32);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;//添加好友回复
            MyTcpServer::getInstance().resend(caPername,respdu);
            free(respdu);//释放
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST://刷新好友请求15
        {
            log << "刷新好友请求";
            char caPername[32] = {'\0'};
            memcpy(caPername, pdu->caData, 32);
            QStringList ret = OpeDB::getInstance().handleFlushFriend(caPername);
            uint uiMsgLen = ret.size() * 32;
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
            for(int i =0; i <ret.size(); i++)
            {
                memcpy((char*)respdu->caMsg + i *32,
                       ret.at(i).toStdString().c_str(),
                       ret.at(i).size());
            }
            write((char*)respdu,respdu->uiPDULen);//发送
            free(respdu);//释放
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST://删除好友请求17
        {
            log << "删除好友请求";
            char caPername[32] = {'\0'};//对方
            char caName[32] = {'\0'};//自己
            memcpy(caName, pdu->caData, 32);
            memcpy(caPername, pdu->caData + 32, 32);
            OpeDB::getInstance().handleDeleteFriend(caPername,caName);
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
            strcpy(respdu->caData,DEL_FRIEND_OK);
            write((char*)respdu,respdu->uiPDULen);//发送给自己
            free(respdu);//释放
            respdu = NULL;
            MyTcpServer::getInstance().resend(caPername,pdu);//发送给好友
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST://私聊请求19
        {
            log << "私聊请求";
            char caPername[32] = {'\0'};
            memcpy(caPername, pdu->caData + 32, 32);
            MyTcpServer::getInstance().resend(caPername,pdu);
            break;

        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST://群聊请求21
        {
            log << "群聊请求";
            char caName[32] = {'\0'};//发送方
            memcpy(caName, pdu->caData, 32);
            QStringList onlineFriend = OpeDB::getInstance().handleFlushFriend(caName);
            QString tmp;
            for(int i =0; i < onlineFriend.size(); i++)
            {
                tmp = onlineFriend.at(i);
                MyTcpServer::getInstance().resend(tmp.toStdString().c_str(),pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST://创建文件夹请求23
        {
            log << "创建文件夹请求";
            QDir dir;
            QString strCurPath = QString("%1").arg((char*)pdu->caMsg);
            bool ret = dir.exists(strCurPath);
            PDU *respdu = NULL;
            if(ret)//当前目录存在
            {
                char caNewDir[32] = {'\0'};
                memcpy(caNewDir,pdu->caData + 32, 32);
                QString strNewPath = strCurPath + "/" + caNewDir;
                log << strNewPath;
                ret = dir.exists(strNewPath);
                if(ret)//创建的文件名已存在
                {
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(respdu->caData, FILE_NAME_EXIST);
                }
                else
                {
                    dir.mkdir(strNewPath);
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(respdu->caData, CREAT_DIR_OK);
                }
            }
            else
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                strcpy(respdu->caData, DIR_NO_EXIST);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST://刷新(查看)文件请求25
        {
            log << "刷新(查看)文件请求";
            char *pCurPath = new char[pdu->uiMsgLen];
            memcpy(pCurPath, pdu->caMsg, pdu->uiMsgLen);
            QDir dir(pCurPath);
            QFileInfoList fileInfoList = dir.entryInfoList();//文件名列表
            int iFileCount = fileInfoList.size();
            PDU *respdu = mkPDU(sizeof(FileInfo)*iFileCount);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;//刷新文件夹回复
            FileInfo *pFileInfo = NULL;
            QString strFileName;
            for(int i = 0; i < iFileCount; i++)
            {
                log << fileInfoList[i].fileName() << " name.size:"
                    << fileInfoList[i].fileName().size() <<" size: "
                    << fileInfoList[i].size();
                if(QString(".") ==fileInfoList[i].fileName() || QString("..") ==fileInfoList[i].fileName())
                {
                    continue;
                }
                pFileInfo = (FileInfo*)respdu->caMsg + i;
                strFileName = fileInfoList[i].fileName();
                memcpy(pFileInfo->caFileName,strFileName.toStdString().c_str(),32);
                if (fileInfoList[i].isDir())
                {
                    log << "是文件夹";
                    pFileInfo->iFileType = 0;
                }
                else if (fileInfoList[i].isFile())
                {
                    log << "是常规文件";
                    pFileInfo->iFileType = 1;
                }
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_REQUEST://删除文件/文件夹请求27
        {
            log << "删除文件/文件夹请求";
            char caName[32] ={'\0'};
            strcpy(caName, pdu->caData);
            //char *pPath = new char[pdu->uiMsgLen];
            //memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg((char*)pdu->caMsg).arg(caName);
            log << strPath;
            QFileInfo fileInfo(strPath);
            bool ret = false;
            if(fileInfo.isDir())
            {
                log << "dir";
                QDir dir;
                dir.setPath(strPath);//设置到文件夹
                ret = dir.removeRecursively();//删除文件夹
            }
            else if(fileInfo.isFile())
            {
                log << "file";
                QDir dir;
                ret = dir.remove(strPath);//删除文件夹
            }
            log << ret;
            PDU *respdu = NULL;
            if(ret)
            {
                respdu = mkPDU(0);
                strcpy(respdu->caData,DEL_DIR_OK);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
            }
            else
            {
                respdu = mkPDU(0);
                strcpy(respdu->caData,DEL_DIR_FAILED);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_REQUEST://重命名文件请求29
        {
            log <<"重命名文件请求";
            char caOldName[32] = {'\0'};
            char caNewName[32] = {'\0'};
            strncpy(caOldName, pdu->caData, 32);
            strncpy(caNewName, pdu->caData+32, 32);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strOldPath = QString("%1/%2").arg(pPath).arg(caOldName);
            QString strNewPath = QString("%1/%2").arg(pPath).arg(caNewName);
            log << "oldPath:" << strOldPath;
            log << "newPath:" << strNewPath;
            QDir dir;
            bool ret = dir.rename(strOldPath, strNewPath);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
            if(ret)
            {
                log << RENAME_OK;
                strcpy(respdu->caData, RENAME_OK);
            }
            else
            {
                log << RENAME_FAILED;
                strcpy(respdu->caData, RENAME_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_REQUEST://进入文件夹请求31
        {
            log << "进入文件夹请求";
            char caEnterName[32] = {'\0'};
            strcpy(caEnterName, pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caEnterName);
            log << strPath;
            QFileInfo fileInfo(strPath);
            PDU *respdu = NULL;
            if (fileInfo.isDir())
            {
                QDir dir(strPath);
                QFileInfoList fileInfoList = dir.entryInfoList();
                int iFileCount = fileInfoList.size();
                respdu = mkPDU(sizeof(FileInfo)*iFileCount);
                respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;//直接刷新
                FileInfo *pFileInfo = NULL;
                QString strFileName;
                for (int i=0; i < iFileCount; i++)
                {
                    if(QString(".") == fileInfoList[i].fileName() || QString("..") == fileInfoList[i].fileName())
                    {
                        continue;
                    }
                    pFileInfo = (FileInfo*)(respdu->caMsg)+i;
                    strFileName = fileInfoList[i].fileName();
                    memcpy(pFileInfo->caFileName,strFileName.toStdString().c_str(),32);
                    if (fileInfoList[i].isDir())
                    {
                        log << "是文件夹";
                        pFileInfo->iFileType = 0;
                    }
                    else if (fileInfoList[i].isFile())
                    {
                        log << "是常规文件";
                        pFileInfo->iFileType = 1;
                    }
                }
            }
            else if (fileInfo.isFile())
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;//进入文件夹回复
                strcpy(respdu->caData, ENTER_DIR_FAILURED);
            }
            log;
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST://上传文件请求33
        {
            log << "上传文件请求";
            char caFileName[32] = {'\0'};
            qint64 fileSize = 0;//
            sscanf(pdu->caData, "%s %lld", caFileName, &fileSize);//拷贝存放地址，“名字 文件大小”，需考的数据，返回文件大小
            QString strPath = QString("%1/%2").arg((char*)pdu->caMsg).arg(caFileName);
            log << strPath;
            m_file.setFileName(strPath);
            if (m_file.open(QIODevice::WriteOnly))//以只写的方式打开文件，若文件不存在，则会自动创建文件
            {
                m_bUpload = true;
                m_iTotal = fileSize;
                m_iRecved = 0;
            }
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST://下载文件请求35
        {
            log << "下载文件请求";
            char caFileName[32] = {'\0'};
            strcpy(caFileName, pdu->caData);
            QString strPath = QString("%1/%2").arg((char*)pdu->caMsg).arg(caFileName);
            log << strPath;
            QFileInfo fileInfo(strPath);
            qint64 fileSize = fileInfo.size();
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;//下载回复
            sprintf(respdu->caData, "%s %lld", caFileName, fileSize);//拷贝存放地址，“名字 文件大小”，需考的数据，返回文件大小
            log << strPath;
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            m_file.setFileName(strPath);
            m_file.open(QIODevice::ReadOnly);
            m_pTimer->start(1000);
            log;

            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_REQUEST://共享文件请求37
        {
            log << "共享文件请求";
            char caSendName[32] = {'\0'};
            int num = 0;
            sscanf(pdu->caData, "%s%d", caSendName, &num);
            int size = num*32;
            PDU *respdu = mkPDU(pdu->uiMsgLen-size);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE;//分享文件通知
            strcpy(respdu->caData, caSendName);
            memcpy(respdu->caMsg, (char*)(pdu->caMsg)+size, pdu->uiMsgLen-size);

            char caRecvName[32] = {'\0'};
            for (int i=0; i<num; i++)
            {
                memcpy(caRecvName, (char*)(pdu->caMsg)+i*32, 32);
                MyTcpServer::getInstance().resend(caRecvName, respdu);
            }
            free(respdu);
            respdu = NULL;

            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;//共享文件回复
            strcpy(respdu->caData, "分享成功");
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND://共享文件通知回复40
        {
            QString strRecvPath = QString("./%1").arg(pdu->caData);
            QString strShareFilePath = QString("%1").arg((char*)(pdu->caMsg));
            int index = strShareFilePath.lastIndexOf('/');
            QString strFileName = strShareFilePath.right(strShareFilePath.size()-index-1);
            strRecvPath = strRecvPath+'/'+strFileName;

            QFileInfo fileInfo(strShareFilePath);
            if (fileInfo.isFile())//常规文件
            {
                QFile::copy(strShareFilePath, strRecvPath);
            }
            else if (fileInfo.isDir())//文件夹
            {
                copyDir(strShareFilePath, strRecvPath);
            }
            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_REQUEST://移动文件请求41
        {
            log << "移动文件请求";
            char caFileName[32] = {'\0'};
            int srcLen = 0;
            int destLen = 0;
            sscanf(pdu->caData, "%d%d%s", &srcLen, &destLen, caFileName);

            char *pSrcPath = new char[srcLen+1];
            char *pDestPath = new char[destLen+1+32];
            memset(pSrcPath, '\0', srcLen+1);//清空
            memset(pDestPath, '\0', destLen+1+32);

            memcpy(pSrcPath, pdu->caMsg, srcLen);
            memcpy(pDestPath, (char*)(pdu->caMsg)+(srcLen+1), destLen);

            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;
            QFileInfo fileInfo(pDestPath);
            if (fileInfo.isDir())
            {
                strcat(pDestPath, "/");//拼接，在后面添加
                strcat(pDestPath, caFileName);

                bool ret = QFile::rename(pSrcPath, pDestPath);//重命名路径
                if (ret)
                {
                    strcpy(respdu->caData, MOVE_FILE_OK);
                }
                else
                {
                    strcpy(respdu->caData, COMMON_ERR);
                }
            }
            else if (fileInfo.isFile())
            {
                strcpy(respdu->caData, MOVE_FILE_FAILURED);
            }

            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;


            break;
        }
        default:
            break;
        }
        free(pdu);//释放
        pdu = NULL;
    }
    else//上传文件
    {
        log << "Uploading";
        QByteArray buff = readAll();
        m_file.write(buff);
        m_iRecved += buff.size();
        PDU *respdu = NULL;
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
        if (m_iTotal == m_iRecved)
        {
            m_file.close();
            m_bUpload = false;
            strcpy(respdu->caData, UPLOAD_FILE_OK);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if (m_iTotal < m_iRecved)
        {
            m_file.close();
            m_bUpload = false;
            strcpy(respdu->caData, UPLOAD_FILE_FAILURED);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;//不能放在外面，会多次发送
        }
    }
}
void MyTcpSocket::clientOffline()//处理下线
{
    OpeDB::getInstance().handleOffline(m_strName.toStdString().c_str());
    emit offline(this);
    log;
}

void MyTcpSocket::sendFileToClient()//发送文件给客户端
{
    m_pTimer->stop();
    if(!m_file.isOpen())
    {
        log << "不是常规文件";
        return;
    }
    char *pData = new char[4096];//每次4096字节效率最高
    qint64 ret = 0;
    while (true)
    {
        ret = m_file.read(pData, 4096);
        if (ret > 0 && ret <= 4096)
        {
            write(pData, ret);
        }
        else if (ret == 0)
        {
            m_file.close();
            break;
        }
        else if (ret < 0)
        {
            log << "发送文件内容给客户端过程中失败";
            m_file.close();
            break;
        }
    }
    delete []pData;
    pData = NULL;
}

void MyTcpSocket::copyDir(QString strSrcDir, QString strDestDir)//拷贝文件
{
    QDir dir;
    dir.mkdir(strDestDir);

    dir.setPath(strSrcDir);
    QFileInfoList fileInfoList = dir.entryInfoList();//获得文件列表信息

    QString srcTmp;//文件目录
    QString destTmp;//拷贝到的路径
    for (int i=0; i<fileInfoList.size(); i++)
    {
        qDebug() << "fileName:" << fileInfoList[i].fileName();
        if (fileInfoList[i].isFile())
        {
            srcTmp = strSrcDir+'/'+fileInfoList[i].fileName();
            destTmp = strDestDir+'/'+fileInfoList[i].fileName();
            QFile::copy(srcTmp, destTmp);
        }
        else if (fileInfoList[i].isDir())
        {
            if (QString(".") == fileInfoList[i].fileName()
                    || QString("..") == fileInfoList[i].fileName())
            {
                continue;
            }
            srcTmp = strSrcDir+'/'+fileInfoList[i].fileName();
            destTmp = strDestDir+'/'+fileInfoList[i].fileName();
            copyDir(srcTmp, destTmp);
        }
    }
}

QString MyTcpSocket::getName()
{
    return m_strName;
}
