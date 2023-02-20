#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "protocol.h"
#include "utils.h"
#include <QTimer>

class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void updateFileList(PDU *pdu);//显示文件夹信息
    void clrarEnterDir();//清空文件夹名
    QString getEnterDir();
    void setDownloadStatus(bool status);//设置下载状态

    qint64 m_iTotal;//文件大小
    qint64 m_iRecved;//文件接收进度
    bool getDownloadStatus();
    QString getSaveFilePath();
    QString getShareFileName();

signals:

public slots:
    void createDir();//创建文件夹
    void flushFile();//刷新文件夹
    void delDir();//删除文件夹
    void renameFile();//重命名文件夹
    void enterDir(const QModelIndex &index);//进入文件夹

    void returnPre();//返回上一级
    void uploadFile();//上传文件请求
    void uploadFileData();//上传文件
    void downloadFile();//下载文件
    void shareFile();//分享文件
    void moveFile();//移动文件
    void selectDestDir();//选择文件目标地址

private:
    QListWidget *m_pBookListW;  //文件夹目录

    QPushButton *m_pReturnPB;   //返回
    QPushButton *m_pCreateDirPB;//创建文件夹
    QPushButton *m_pDelDirPB;   //删除文件/文件夹
    QPushButton *m_pRenamePB;   //重命名文件夹
    QPushButton *m_pFlushFilePB;//刷新文件夹

    QPushButton *m_pUploadPB;   //上传文件
    QPushButton *m_pDelFilePB;  //下载文件
    QPushButton *m_pShareFilePB;//分享文件
    QPushButton *m_pMoveFilePB; //移动文件/文件夹
    QPushButton *m_pSelectDirPB;//移动的目标目录

    QString m_strEnterDir;//进入文件夹名

    QString m_strUploadFilePath;//上传文件路径
    QTimer *m_pTimer;//定时器

    QString m_strSaveFilePath;//下载保存路径
    bool m_bDownload;//是否为下载状态

    QString m_strShareFileName;//分享文件名

    QString m_strMoveFileName;//需要移动的文件名
    QString m_strMoveFilePath;//需要移动的文件名的路径
    QString m_strDestDir;//目标地址
};

#endif // BOOK_H
