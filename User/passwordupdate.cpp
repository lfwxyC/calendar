#include "passwordupdate.h"
#include "ui_passwordupdate.h"

#include <QMessageBox>
#include <QtSql/QSqlQuery>

passwordUpdate::passwordUpdate(QWidget *parent) : // 修改密码
    QDialog(parent),
    ui(new Ui::passwordUpdate)
{
    ui->setupUi(this);

    // 设置数据库
    db=QSqlDatabase::database("conn");
    query=new QSqlQuery(db);
}

passwordUpdate::~passwordUpdate()
{
    delete ui;
    delete query;
}

void passwordUpdate::on_okBt_clicked()
{
    // 获取用户输入
    QString newPassword=ui->newPasswordLine->text();
    QString confirm=ui->confirmPasswordLine->text();

    if(newPassword!=confirm)
    {
        QMessageBox::warning(this,"warning","两次输入的密码不一致！",QMessageBox::Ok);
        ui->newPasswordLine->clear();
        ui->confirmPasswordLine->clear();
    }
    else {
        QMessageBox::information(this,"info","修改成功",QMessageBox::Ok);

        QString update=QString("update user set password='%1' where phone='%2'").arg(newPassword).arg(phone); // 根据手机号修改密码
        query->exec(update);
        this->close();
    }
}

void passwordUpdate::sendPhone(const QString &phone) // 从上一个页面接收参数
{
    this->phone=phone;
}
