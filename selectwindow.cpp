#include "selectwindow.h"
#include "ui_selectwindow.h"
#include "privatewindow.h"
#include "loginwindow.h"
#include "publicwindow.h"
#include "publicshow.h"
#include "historywindow.h"

#include <QtSql/QSqlQuery>
#include <QInputDialog>
#include <QMessageBox>
#include <QDate>
#include <QSqlQueryModel>

SelectWindow::SelectWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SelectWindow)
{
    ui->setupUi(this);

    // 设置数据库
    db=QSqlDatabase::database("conn");
    query=new QSqlQuery(db);

    hide();
}

SelectWindow::~SelectWindow()
{
    delete ui;
    delete query;
}

void SelectWindow::hide() // 隐藏修改密码的按钮
{
    ui->newPasswordLine->hide();
    ui->confirmPasswordLine->hide();
    ui->okBt->hide();
    ui->cancelBt->hide();
}

void SelectWindow::on_privateBt_clicked() // 显示私人页面，关闭个人主页
{
    PrivateWindow *privateWin=new PrivateWindow;

    privateWin->sendUser(user);
    privateWin->show();
    this->close();
}

void SelectWindow::on_publicBt_clicked() // 显示公开页面，关闭个人主页
{
    publicWindow *publicWin=new publicWindow;

    publicWin->sendUser(user);
    publicWin->show();
    this->close();
}

void SelectWindow::sendUser(const QString &user) // 从上一个页面接收参数
{
    QString select=QString("select name from user where email='%1'").arg(user);
    QString name;
    this->user=user;

    query->exec(select);
    while (query->next()) {
        name=query->value(0).toString();
    }

    // 设置昵称和邮箱
    ui->userLabel->setText(name);
    ui->emailLabel->setText(user);

    // 从数据库中查找三天及以内的私人待办
    QDate today=QDate::currentDate();
    QDate day=QDate::currentDate().addDays(3);
    select=QString("select * from privateTodo where tdate<='%1'and tuser='%2'and tdate>='%3'").arg(day.toString("yyyy-MM-dd")).arg(user).arg(today.toString("yyyy-MM-dd"));

    // 设置私人待办的tableview
    QSqlQueryModel *model=new QSqlQueryModel(ui->privateTV);
    model->setQuery(select,this->db);
    model->setHeaderData(0,Qt::Horizontal,tr("标签"));
    model->setHeaderData(1,Qt::Horizontal,tr("主题"));
    model->setHeaderData(2,Qt::Horizontal,tr("日期"));
    model->setHeaderData(3,Qt::Horizontal,tr("时间"));
    model->setHeaderData(6,Qt::Horizontal,tr("重复"));
    ui->privateTV->setModel(model);
    ui->privateTV->setColumnHidden(4,true);
    ui->privateTV->setColumnHidden(5,true);

    // 从数据库中查找三天及以内的公开待办
    select=QString("select publicTodo.* from publicTodo,creator_user where id=tid and date<='%1'and memberEmail='%2'and date>='%3'").arg(day.toString("yyyy-MM-dd")).arg(user).arg(today.toString("yyyy-MM-dd"));

    // 设置公开待办的tableview
    model=new QSqlQueryModel(ui->publicTV);
    model->setQuery(select,this->db);
    model->setHeaderData(0,Qt::Horizontal,tr("标签"));
    model->setHeaderData(1,Qt::Horizontal,tr("主题"));
    model->setHeaderData(2,Qt::Horizontal,tr("日期"));
    model->setHeaderData(3,Qt::Horizontal,tr("时间"));
    model->setHeaderData(4,Qt::Horizontal,tr("发起者"));
    model->setHeaderData(6,Qt::Horizontal,tr("具体描述"));
    model->setHeaderData(7,Qt::Horizontal,tr("重复"));
    ui->publicTV->setModel(model);
    ui->publicTV->setColumnHidden(5,true);
}

void SelectWindow::on_logoutBt_clicked() // 登出
{
    LoginWindow *loginWin=new LoginWindow;
    loginWin->show();
    this->close();
}

void SelectWindow::on_updateNameBt_clicked()
{
    bool ok;
    QString updateName=QInputDialog::getText(this,tr("修改昵称"),tr("请输入修改后的昵称"),QLineEdit::Normal,0,&ok);

    if(ok&&!updateName.isEmpty())
    {
        ui->userLabel->setText(updateName);
        QString update=QString("update user set name='%1' where email='%2'").arg(updateName).arg(user); // 更新用户昵称
        query->exec(update);
    }
}

void SelectWindow::on_updatePasswordBt_clicked() // 验证原密码
{
    bool ok;
    QString password=QInputDialog::getText(this,tr("修改密码"),tr("请输入原密码"),QLineEdit::Password,0,&ok);

    if(ok&&!password.isEmpty())
    {
        // 获取用户输入
        QString select=QString("SELECT * FROM user WHERE email='%1'").arg(user);
        QString selectPassword;

        query->exec(select);
        while(query->next())
        {
            selectPassword=query->value(2).toString();
        }

        if(password!=selectPassword)
        {
            QMessageBox::warning(this,"warning","密码错误！",QMessageBox::Ok);
        }
        else { // 重置密码
            // 显示修改密码的按钮
            ui->newPasswordLine->show();
            ui->confirmPasswordLine->show();
            ui->okBt->show();
            ui->cancelBt->show();

            ui->newPasswordLine->setFocus(); //光标显示在输入新密码的行

            // 按enter键自动换行
            connect(ui->newPasswordLine, SIGNAL(returnPressed()), this, SLOT(next()));
            connect(ui->confirmPasswordLine, SIGNAL(returnPressed()), ui->okBt, SIGNAL(clicked()), Qt::UniqueConnection);

            // 隐藏待办
            ui->privateLabel->hide();
            ui->publicLabel->hide();
            ui->privateTV->hide();
            ui->publicTV->hide();
        }
    }
}

void SelectWindow::on_okBt_clicked() // 修改密码
{
    // 获取用户输入
    QString newPassword=ui->newPasswordLine->text();
    QString confirm=ui->confirmPasswordLine->text();

    if(newPassword!=confirm)
    {
        QMessageBox::warning(this,"warning","两次输入的密码不一致！",QMessageBox::Ok);

        // 清空输入框
        ui->newPasswordLine->clear();
        ui->confirmPasswordLine->clear();
        ui->newPasswordLine->setFocus(); // 光标显示在输入新密码的行
    }
    else {
        QMessageBox::information(this,"info","修改成功",QMessageBox::Ok);

        QString update=QString("update user set password='%1' where email='%2'").arg(newPassword).arg(user); // 在数据库中更新密码
        query->exec(update);

        hide();

        // 显示待办
        ui->privateLabel->show();
        ui->publicLabel->show();
        ui->privateTV->show();
        ui->publicTV->show();
    }
}

void SelectWindow::on_publicTV_doubleClicked(const QModelIndex &index) // 双击公开待办可查看相应行的具体内容
{
    ui->publicTV->setSelectionBehavior(QAbstractItemView::SelectRows); // 点击单元格时选中整行

    // 获取选中行的数据
    int currentRow = ui->publicTV->currentIndex().row();
    QAbstractItemModel *modelTV = ui->publicTV->model ();
    QModelIndex idIndex = modelTV->index(currentRow,5);
    QModelIndex dateIndex=modelTV->index(currentRow,2);
    QVariant idData = modelTV->data(idIndex);
    QVariant dateData=modelTV->data(dateIndex);
    int selectId=idData.toInt();
    QDate selectDate=dateData.toDate();

    // 显示公开待办的具体内容
    publicShow *publicShowDia=new publicShow;
    publicShowDia->show();
    publicShowDia->sendInfo(user,selectId,selectDate);
}

void SelectWindow::on_historyBt_clicked() // 显示历史待办页面，关闭个人主页
{
    historyWindow *historyWin=new historyWindow;
    historyWin->show();
    historyWin->sendUser(user);
    this->close();
}

void SelectWindow::next()
{
    ui->confirmPasswordLine->setFocus(); // 光标显示在确认密码的行
}

void SelectWindow::on_cancelBt_clicked() // 取消修改密码
{
    // 清空输入框
    ui->newPasswordLine->clear();
    ui->confirmPasswordLine->clear();

    hide();

    // 显示待办
    ui->privateLabel->show();
    ui->publicLabel->show();
    ui->privateTV->show();
    ui->publicTV->show();
}
