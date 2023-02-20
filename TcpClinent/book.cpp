#include "book.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include "opewidget.h"
#include "sharefile.h"

Book::Book(QWidget *parent) : QWidget(parent)
{
    m_pBookListW = new QListWidget;//文件夹目录
    m_bDownload = false;
    m_pTimer = new QTimer;

    //文件夹
    m_pReturnPB = new QPushButton("返回");
    m_pCreateDirPB = new QPushButton("创建文件夹");
    m_pDelDirPB = new QPushButton("删除文件/文件夹");
    m_pRenamePB = new QPushButton("重命名文件夹");
    m_pFlushFilePB = new QPushButton("刷新文件夹");
    QVBoxLayout *pDirVBL = new QVBoxLayout;//垂直布局
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pRenamePB);
    pDirVBL->addWidget(m_pFlushFilePB);

    //文件
    m_pUploadPB = new QPushButton("上传文件");
    m_pDelFilePB = new QPushButton("下载文件");
    m_pShareFilePB = new QPushButton("分享文件");
    m_pMoveFilePB = new QPushButton("移动文件/文件夹");
    m_pSelectDirPB = new QPushButton("目标目录");
    m_pSelectDirPB->setEnabled(false);//不可选

    QVBoxLayout *pFileVBL = new QVBoxLayout;//垂直布局
    pFileVBL->addWidget(m_pUploadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);
    pFileVBL->addWidget(m_pMoveFilePB);
    pFileVBL->addWidget(m_pSelectDirPB);

    QHBoxLayout *pMain = new QHBoxLayout;//水平布局
    pMain->addWidget(m_pBookListW);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain);//展示

    QObject::connect(m_pCreateDirPB,&QPushButton::clicked,
                     this,&Book::createDir);
    QObject::connect(m_pFlushFilePB,&QPushButton::clicked,
                     this,&Book::flushFile);
    QObject::connect(m_pDelDirPB,&QPushButton::clicked,
                     this,&Book::delDir);
    QObject::connect(m_pRenamePB,&QPushButton::clicked,
                     this,&Book::renameFile);
    QObject::connect(m_pBookListW,&QListWidget::doubleClicked,//双击
                     this,&Book::enterDir);
    QObject::connect(m_pReturnPB,&QPushButton::clicked,
                     this,&Book::returnPre);
    QObject::connect(m_pUploadPB,&QPushButton::clicked,
                     this,&Book::uploadFile);
    QObject::connect(m_pTimer,&QTimer::timeout,
                     this,&Book::uploadFileData);
    QObject::connect(m_pDelFilePB,&QPushButton::clicked,
                     this,&Book::downloadFile);
    QObject::connect(m_pShareFilePB,&QPushButton::clicked,
                     this,&Book::shareFile);
    QObject::connect(m_pMoveFilePB,&QPushButton::clicked,
                     this,&Book::moveFile);
    connect(m_pSelectDirPB, &QPushButton::clicked,
            this, &Book::selectDestDir);
}

void Book::updateFileList(PDU *pdu)//显示文件夹信息
{
    if(pdu == NULL)
    {
        log << "pdu == NULL";
        return;
    }
    m_pBookListW->clear();
    /*QListWidgetItem *pItemTmp = NULL;
    int row = m_pBookListW->count();
    while (m_pBookListW->count() > 0)
    {
        pItemTmp = m_pBookListW->item(row - 1);
        m_pBookListW->removeItemWidget(pItemTmp);
        delete pItemTmp;
        row = row - 1;
    }*/

    FileInfo *pFileInfo = NULL;
    int iCout = pdu->uiMsgLen / sizeof (FileInfo);
    for(int i = 0; i < iCout; i++)
    {
        pFileInfo = (FileInfo*)pdu->caMsg + i;
        log << pFileInfo->caFileName << " " << pFileInfo->iFileType;
        QListWidgetItem *pItem = new QListWidgetItem;
        if(pFileInfo->iFileType == 0)
        {
            pItem->setIcon(QIcon(QPixmap(":/map/dir.jpg")));
        }
        else
        {
            pItem->setIcon(QIcon(QPixmap(":/map/reg.jpg")));
        }
        pItem->setText(pFileInfo->caFileName);
        m_pBookListW->addItem(pItem);
    }
}

void Book::createDir()//新建文件夹
{
    QString strNewDir = QInputDialog::getText(this,"新建文件夹","输入新文件名");
    if(strNewDir.isEmpty())
    {
        QMessageBox::warning(this,"新建文件夹","新文件夹名不能为空");
    }
    else
    {
        if(strNewDir.size() > 32)
        {
            QMessageBox::warning(this,"新建文件夹","新文件夹名字不能超过32个字符");
        }
        else
        {
            QString strName = TcpClient::getInstance().loginName();
            QString strCurPath = TcpClient::getInstance().curPath();
            PDU *pdu = mkPDU(strCurPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;//创建文件夹请求
            memcpy(pdu->caData, strName.toStdString().c_str(), strName.size());
            memcpy(pdu->caData + 32, strNewDir.toStdString().c_str(), strNewDir.size());
            memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
            TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//发送
            log << pdu->caData;
            free(pdu);
            pdu = NULL;
        }
    }
}

void Book::flushFile()//刷新文件夹
{
    QString strCurPath = TcpClient::getInstance().curPath();
    PDU *pdu = mkPDU(strCurPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;//刷新(查看)文件请求
    memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//发送
    log << pdu->caData;
    free(pdu);
    pdu = NULL;
}

void Book::delDir()//删除文件夹
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if( pItem == NULL)
    {
        QMessageBox::warning(this,"删除文件/文件夹","请选择需要删除的文件/文件夹");
    }
    else
    {
        int ret = QMessageBox::information(this, "删除文件夹",QString("删除后文件/文件夹将不可恢复是否继续？"), QMessageBox::Yes, QMessageBox::No);
        if(ret == QMessageBox::Yes)
        {
            QString strDelName = pItem->text();
            PDU *pdu = mkPDU(strCurPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REQUEST;//删除文件/文件夹请求
            memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
            strcpy(pdu->caData, strDelName.toStdString().c_str());
            log << (char*)pdu->caMsg << pdu->caData;
            TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//发送
            log << pdu->caData;
            free(pdu);
            pdu = NULL;
        }
    }
}

void Book::renameFile()//重命名文件
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if( pItem == NULL)
    {
        QMessageBox::warning(this,"重命名文件夹","请选择需要重命名的文件");
    }
    else
    {
        QString strOldName = pItem->text();
        QString strNewName = QInputDialog::getText(this, "重命名文件", "请输入新的文件名");
        if (!strNewName.isEmpty())
        {
            PDU *pdu = mkPDU(strCurPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
            strcpy(pdu->caData, strOldName.toStdString().c_str());
            strcpy(pdu->caData+32, strNewName.toStdString().c_str());
            memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
            log << pdu->caData << " " << (char*)pdu->caMsg;
            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }
        else
        {
            QMessageBox::warning(this, "重命名文件", "新文件名不能为空");
        }
    }
}

void Book::enterDir(const QModelIndex &index)//进入文件夹
{
    log <<"进入文件夹";
    m_strEnterDir = index.data().toString();
    QString strCurPath = TcpClient::getInstance().curPath();
    PDU *pdu = mkPDU(strCurPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    strcpy(pdu->caData, m_strEnterDir.toStdString().c_str());
    memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::clrarEnterDir()//清空进入路径
{
    m_strEnterDir.clear();
}

QString Book::getEnterDir()
{
    return m_strEnterDir;
}

void Book::setDownloadStatus(bool status)//设置下载状态
{
    m_bDownload =status;
}

bool Book::getDownloadStatus()
{
    return m_bDownload;
}

QString Book::getSaveFilePath()
{
    return m_strSaveFilePath;
}

QString Book::getShareFileName()
{
    return m_strShareFileName;
}

void Book::returnPre()//返回上一级
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QString strRootPath = "./" + TcpClient::getInstance().loginName();
    if(strCurPath == strRootPath)
    {
        QMessageBox::warning(this,"返回","返回失败:已在初始目录");
    }
    else
    {
        int index = strCurPath.lastIndexOf('/');
        strCurPath.remove(index,strCurPath.size()-index);
        TcpClient::getInstance().setCurPath(strCurPath);
        m_pBookListW->clear();
        flushFile();
    }
}

void Book::uploadFile()//上传文件请求
{
    m_strUploadFilePath = QFileDialog::getOpenFileName();
    log << m_strUploadFilePath;
    if (!m_strUploadFilePath.isEmpty())
    {
        int index = m_strUploadFilePath.lastIndexOf('/');
        QString strFileName = m_strUploadFilePath.right(m_strUploadFilePath.size()-index-1);
        log << strFileName;
        QFile file(m_strUploadFilePath);
        qint64 fileSize = file.size();//获得文件大小
        QString strCurPath = TcpClient::getInstance().curPath();
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        sprintf(pdu->caData, "%s %lld", strFileName.toStdString().c_str(), fileSize);
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;

        m_pTimer->start(1000);
    }
    else
    {
        QMessageBox::warning(this, "上传文件", "上传文件名字不能为空");
    }
}

void Book::uploadFileData()//上传文件
{
    m_pTimer->stop();
    QFile file(m_strUploadFilePath);
    if (!file.open(QIODevice::ReadOnly))//以只读模式打开
    {
        QMessageBox::warning(this, "上传文件", "打开文件失败");
        return;
    }
    char *pBuffer = new char[4096];//每次4096字节效率最高
    qint64 ret = 0;
    while (true)
    {
        ret = file.read(pBuffer, 4096);//每次最多读取4096字节
        if (ret > 0 && ret <= 4096)
        {
            TcpClient::getInstance().getTcpSocket().write(pBuffer, ret);//发送
        }
        else if (ret == 0)
        {
            break;
        }
        else
        {
            QMessageBox::warning(this, "上传文件", "上传文件失败:读取文件失败");
            break;
        }
    }
    file.close();
    delete []pBuffer;
    pBuffer = NULL;
}

void Book::downloadFile()//下载
{
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if( pItem == NULL)
    {
        QMessageBox::warning(this,"下载","请选择需要下载的文件");
    }
    else
    {
        QString strSaveFilePath = QFileDialog::getSaveFileName();//获得保存路径
        if(strSaveFilePath.isEmpty())
        {
            QMessageBox::warning(this,"下载","保存路径不能为空");
            m_strSaveFilePath.clear();
        }
        else
        {
            m_strSaveFilePath = strSaveFilePath;
        }
        QString strCurPath = TcpClient::getInstance().curPath();
        QString strFileName = pItem->text();//文件名
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;//下载文件请求
        memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        strcpy(pdu->caData, strFileName.toStdString().c_str());
        log << (char*)pdu->caMsg << pdu->caData;
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//发送
        //不需要释放
    }
}

void Book::shareFile()//分享文件
{
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (NULL == pItem)
    {
        QMessageBox::warning(this, "分享文件", "请选择要分享的文件");
        return;
    }
    else
    {
        m_strShareFileName = pItem->text();
    }
    Friend *pFriend = OpeWidget::getInstance().getFrined();
    QListWidget *pFriendList = pFriend->getFriendList();
    ShareFile::getInstance().updateFriend(pFriendList);
    if (ShareFile::getInstance().isHidden())
    {
        ShareFile::getInstance().show();
    }
}

void Book::moveFile()//移动文件
{
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();
    if (NULL != pCurItem)
    {
        m_strMoveFileName = pCurItem->text();
        QString strCutPath = TcpClient::getInstance().curPath();
        m_strMoveFilePath = strCutPath+'/'+m_strMoveFileName;

        m_pSelectDirPB->setEnabled(true);//可选
    }
    else
    {
        QMessageBox::warning(this, "移动文件", "请选择要移动的文件");
    }
}

void Book::selectDestDir()//选择目标目录
{
    do{
        QListWidgetItem *pCurItem = m_pBookListW->currentItem();
        if (NULL != pCurItem)
        {
            if(pCurItem->text() == m_strMoveFileName)
            {
                QMessageBox::warning(this, "移动文件", "请选择目标路径");
                continue;
            }
            QString strDestDir = pCurItem->text();
            QString strCutPath = TcpClient::getInstance().curPath();
            m_strDestDir = strCutPath+'/'+strDestDir;

            int srcLen = m_strMoveFilePath.size();//路径长度
            int destLen = m_strDestDir.size();//目标路径长度
            PDU *pdu = mkPDU(srcLen+destLen+2);
            pdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;//移动文件请求

            sprintf(pdu->caData, "%d %d %s", srcLen, destLen, m_strMoveFileName.toStdString().c_str());//拷贝 c语言
            memcpy((char*)pdu->caMsg, m_strMoveFilePath.toStdString().c_str(), srcLen);
            memcpy((char*)(pdu->caMsg)+(srcLen+1), m_strDestDir.toStdString().c_str(), destLen);

            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }
        else
        {
            QMessageBox::warning(this, "移动文件", "请选择要移动的文件");
        }
        m_pSelectDirPB->setEnabled(false);
        break;
    }while(true);
}



