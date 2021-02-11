#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "signupwindow.h"
#include "selectwindow.h"
#include "passwordupdate.h"

#include <QMessageBox>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlTableModel>
#include <QMessageBox>
#include <QShortcut>
#include <QInputDialog>

LoginWindow::LoginWindow(QWidget *parent) : // 登录页面
    QMainWindow(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    ui->emailLine->setFocus(); // 光标显示在输入邮箱的行

    // 设置数据库
    db=QSqlDatabase::database("conn");
    query=new QSqlQuery(db);

    // 建立索引
    QString index=QString("create index email_index on user(email)");
    query->exec(index);

    connect(ui->emailLine, SIGNAL(returnPressed()), this, SLOT(next())); // 按enter键自动跳到输入密码的行
    connect(ui->passwordLine, SIGNAL(returnPressed()), ui->loginBt, SIGNAL(clicked()), Qt::UniqueConnection); // 按enter键登录
}

LoginWindow::~LoginWindow()
{
    delete ui;
    delete query;
}

void LoginWindow::on_signUpBt_clicked() // 显示注册页面，关闭登录页面
{
    SignUpWindow *signUpWin=new SignUpWindow;
    signUpWin->show();
    this->close();
}

void LoginWindow::on_loginBt_clicked()
{
    // 获取用户输入
    QString email=ui->emailLine->text();
    QString password=ui->passwordLine->text();

    if(email==NULL)
    {
        QMessageBox::warning(this,"warning","电子邮箱不能为空！",QMessageBox::Ok);
    }
    else if(password==NULL)
    {
        QMessageBox::warning(this,"warning","密码不能为空！",QMessageBox::Ok);
    }
    else // 输入的邮箱和密码均不为空
    {
        // 从数据库中寻找输入的邮箱对应的信息
        QString select=QString("SELECT * FROM user WHERE email='%1'").arg(email);
        QString selectEmail,selectPassword;

        query->exec(select);
        while(query->next())
        {
            selectEmail=query->value(0).toString();
            selectPassword=query->value(2).toString();
        }
        if(selectEmail==NULL)
        {
            QMessageBox::warning(this,"warning","该电子邮箱未注册！",QMessageBox::Ok);
        }
        else if(password!=selectPassword)
        {
            QMessageBox::warning(this,"warning","密码错误！",QMessageBox::Ok);
            ui->passwordLine->clear();
        }
        else // 登录成功
        {
            SelectWindow *selectWin=new SelectWindow;
            selectWin->show();
            selectWin->sendUser(email);
            this->close();
        }
     }
}

void LoginWindow::next()
{
    ui->passwordLine->setFocus(); // 光标显示在输入密码的行
}

void LoginWindow::on_pushButton_clicked() // 忘记密码
{
    QString user=ui->emailLine->text();

    if(user==NULL)
    {
        QMessageBox::warning(this,"warning","请先输入邮箱！",QMessageBox::Ok);
        return;
    }

    // 验证手机号
    QString select=QString("SELECT * FROM user WHERE email='%1'").arg(user);
    QString selectPhone,selectEmail;

    query->exec(select);
    while(query->next())
    {
        selectEmail=query->value(0).toString();
        selectPhone=query->value(3).toString();
    }

    if(selectEmail==NULL)
    {
        QMessageBox::warning(this,"warning","该邮箱未注册！",QMessageBox::Ok);
        return;
    }

    bool ok;
    QString phone=QInputDialog::getText(this,tr("验证手机号"),tr("请输入电话号码"),QLineEdit::Normal,0,&ok);

    if(ok&&!phone.isEmpty())
    {        
        if(phone!=selectPhone)
        {
            QMessageBox::warning(this,"warning","手机号错误！",QMessageBox::Ok);
        }
        else { // 验证成功，重置密码
            passwordUpdate *passwordUpdateDia=new passwordUpdate;
            passwordUpdateDia->show();
            passwordUpdateDia->sendPhone(phone);
        }
    }
}
