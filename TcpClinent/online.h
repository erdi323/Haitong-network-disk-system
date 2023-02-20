#ifndef ONLINE_H
#define ONLINE_H

//显示在线用户界面
#include <QWidget>
#include "protocol.h"
#include "utils.h"
namespace Ui {
class Online;
}

class Online : public QWidget
{
    Q_OBJECT

public:
    explicit Online(QWidget *parent = nullptr);
    ~Online();

    void showUser(PDU *pdu);

private slots:
    void on_pushButton_addfriend_clicked();

private:
    Ui::Online *ui;
};

#endif // ONLINE_H
