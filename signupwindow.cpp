#include "signupwindow.h"
#include "ui_signupwindow.h"
#include "loginwindow.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlTableModel>
#include <QMessageBox>

SignUpWindow::SignUpWindow(QWidget *parent) : // 注册页面
    QMainWindow(parent),
    ui(new Ui::SignUpWindow)
{
    ui->setupUi(this);

    // 设置数据库
    db=QSqlDatabase::database("conn");
    query=new QSqlQuery(db);

    // 创建索引
    QString index=QString("create index email_index on user(email)");
    query->exec(index);

    // 按enter键自动换行
    connect(ui->EmailLine, SIGNAL(returnPressed()), this, SLOT(next()));
    connect(ui->phoneLine, SIGNAL(returnPressed()), this, SLOT(next()));
    connect(ui->nameLine, SIGNAL(returnPressed()), this, SLOT(next()));
    connect(ui->PasswordLine, SIGNAL(returnPressed()), this, SLOT(next()));
    connect(ui->ConfirmLine, SIGNAL(returnPressed()), ui->SignUpBt, SIGNAL(clicked()), Qt::UniqueConnection); // 按enter键注册
}

SignUpWindow::~SignUpWindow()
{
    delete ui;
    delete query;
}

void SignUpWindow::on_SignUpBt_clicked()
{
    // 获取用户输入
    QString email=ui->EmailLine->text();
    QString phone=ui->phoneLine->text();
    QString name=ui->nameLine->text();
    QString password=ui->PasswordLine->text();
    QString confirm=ui->ConfirmLine->text();

    if(email==NULL)
    {
        QMessageBox::warning(this,"warning","电子邮箱不能为空！",QMessageBox::Ok);
        ui->EmailLine->setFocus();
    }
    else if (!email.contains("@")) {
        QMessageBox::warning(this,"warning","电子邮箱格式不正确！",QMessageBox::Ok);
        ui->EmailLine->setFocus();
    }
    else if (phone==NULL) {
        QMessageBox::warning(this,"warning","电话不能为空！",QMessageBox::Ok);
        ui->phoneLine->setFocus();
    }
    else if (phone.length()!=11) {
        QMessageBox::warning(this,"warning","电话长度不正确！",QMessageBox::Ok);
        ui->phoneLine->setFocus();
    }
    else if(name==NULL)
    {
        QMessageBox::warning(this,"warning","昵称不能为空！",QMessageBox::Ok);
        ui->nameLine->setFocus();
    }
    else if(password==NULL)
    {
        QMessageBox::warning(this,"warning","密码不能为空！",QMessageBox::Ok);
        ui->PasswordLine->setFocus();
    }
    else if(confirm==NULL)
    {
        QMessageBox::warning(this,"warning","请确认密码！",QMessageBox::Ok);
        ui->ConfirmLine->setFocus();
    }
    else if(password!=confirm)
    {
        QMessageBox::warning(this,"warning","两次输入的密码不一致！",QMessageBox::Ok);
        ui->PasswordLine->clear();
        ui->ConfirmLine->clear();
        ui->PasswordLine->setFocus();
    }
    else // 输入格式均正确
    {
        QString select=QString("SELECT * FROM user WHERE email='%1'").arg(email);
        QString insertEmail;

        query->exec(select);
        while(query->next())
        {
            insertEmail=query->value(0).toString();
            if(email==insertEmail)
            {
                QMessageBox::warning(this,"warning","邮箱已存在！",QMessageBox::Ok);
            }            
        }
        if(insertEmail==NULL)
        {
            QString insert=QString("INSERT INTO user VALUES('%1','%2','%3','%4')").arg(email).arg(name).arg(password).arg(phone);
            query->exec(insert);

            // 跳转到登录页面
            QMessageBox::information(this,"info","注册成功",QMessageBox::Ok);
            LoginWindow *loginWin=new LoginWindow;
            loginWin->show();
            this->close();
        }
     }
}

void SignUpWindow::on_backBt_clicked() // 返回登录页面
{
    LoginWindow *loginWin=new LoginWindow;
    loginWin->show();
    this->close();
}

void SignUpWindow::next() // 实现自动跳转
{
    QLineEdit *lineEdit = (QLineEdit *)sender();

    if (lineEdit == ui->EmailLine) {
        ui->phoneLine->setFocus();
    } else if (lineEdit == ui->phoneLine) {
        ui->nameLine->setFocus();
    } else if (lineEdit == ui->nameLine) {
        ui->PasswordLine->setFocus();
    } else if (lineEdit == ui->PasswordLine) {
        ui->ConfirmLine->setFocus();
    }
}
