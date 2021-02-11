#include "publicwindow.h"
#include "ui_publicwindow.h"
#include "selectwindow.h"
#include "readonlydelegate.h"
#include "publicinsert.h"
#include "publicupdate.h"
#include "publicshow.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQueryModel>
#include <QSqlTableModel>
#include <QInputDialog>
#include <QStandardItem>
#include <QDebug>
#include <QMessageBox>

publicWindow::publicWindow(QWidget *parent) : // 公开页面
    QMainWindow(parent),
    ui(new Ui::publicWindow)
{
    ui->setupUi(this);

    // 设置数据库
    db=QSqlDatabase::database("conn");
    query=new QSqlQuery(db);

    // 只能设置今天之后的待办
    QDate today=QDate::currentDate();
    ui->createCW->setMinimumDate(today);
    ui->joinCW->setMinimumDate(today);

    connect(ui->createTV,SIGNAL(clicked(const QModelIndex &)),this,SLOT(onCreateTVClicked()));
    connect(ui->joinTV,SIGNAL(clicked(const QModelIndex &)),this,SLOT(onJoinTVClicked()));

    // 将周六日的颜色去掉
    ui->createCW->setWeekdayTextFormat(Qt::Saturday,ui->createCW->weekdayTextFormat(Qt::Monday));
    ui->createCW->setWeekdayTextFormat(Qt::Sunday,ui->createCW->weekdayTextFormat(Qt::Monday));
    ui->createCW->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);

    ui->joinCW->setWeekdayTextFormat(Qt::Saturday,ui->joinCW->weekdayTextFormat(Qt::Monday));
    ui->joinCW->setWeekdayTextFormat(Qt::Sunday,ui->joinCW->weekdayTextFormat(Qt::Monday));
    ui->joinCW->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
}

publicWindow::~publicWindow()
{
    delete ui;
    delete query;
}

void publicWindow::setCreateTV(const QString &str) // 设置发起待办的tableview
{
    QSqlQueryModel *model=new QSqlQueryModel(ui->createTV);

    model->setQuery(str,this->db);
    model->setHeaderData(0,Qt::Horizontal,tr("标签"));
    model->setHeaderData(1,Qt::Horizontal,tr("主题"));
    model->setHeaderData(2,Qt::Horizontal,tr("日期"));
    model->setHeaderData(3,Qt::Horizontal,tr("时间"));
    model->setHeaderData(6,Qt::Horizontal,tr("具体描述"));
    model->setHeaderData(7,Qt::Horizontal,tr("重复"));
    ui->createTV->setModel(model);
    ui->createTV->setColumnHidden(4,true);
    ui->createTV->setColumnHidden(5,true);

    // 获取选中待办的id
    QAbstractItemModel *modelTV = ui->createTV->model ();
    QModelIndex idIndex = modelTV->index(0,5); // 默认显示第一条待办的参与成员
    QVariant idData = modelTV->data(idIndex);
    QString selectId=idData.toString();
    QString select=QString("select * from creator_user where tid='%1'").arg(selectId);

    // 显示选中待办的参与成员
    model=new QSqlQueryModel(ui->memberTV);
    model->setQuery(select,this->db);
    model->setHeaderData(1,Qt::Horizontal,tr("成员昵称"));
    model->setHeaderData(2,Qt::Horizontal,tr("成员邮箱"));
    ui->memberTV->setModel(model);
    ui->memberTV->setColumnHidden(0,true);
}

void publicWindow::setJoinTV(const QString &str) // 设置参与待办的tableview
{
    QSqlQueryModel *model=new QSqlQueryModel(ui->joinTV);

    model->setQuery(str,this->db);
    model->setHeaderData(0,Qt::Horizontal,tr("标签"));
    model->setHeaderData(1,Qt::Horizontal,tr("主题"));
    model->setHeaderData(2,Qt::Horizontal,tr("日期"));
    model->setHeaderData(3,Qt::Horizontal,tr("时间"));
    model->setHeaderData(4,Qt::Horizontal,tr("发起者"));
    model->setHeaderData(6,Qt::Horizontal,tr("具体描述"));
    model->setHeaderData(7,Qt::Horizontal,tr("重复"));
    ui->joinTV->setModel(model);
    ui->joinTV->setColumnHidden(5,true);

    // 获取选中待办的id
    QAbstractItemModel *modelTV = ui->joinTV->model ();
    QModelIndex idIndex = modelTV->index(0,5); // 默认显示第一条待办的参与成员
    QVariant idData = modelTV->data(idIndex);
    QString selectId=idData.toString();
    QString select=QString("select * from creator_user where tid='%1'").arg(selectId);

    // 显示选中待办的参与成员
    model=new QSqlQueryModel(ui->joinMemberTV);
    model->setQuery(select,this->db);
    model->setHeaderData(1,Qt::Horizontal,tr("成员昵称"));
    model->setHeaderData(2,Qt::Horizontal,tr("成员邮箱"));
    ui->joinMemberTV->setModel(model);
    ui->joinMemberTV->setColumnHidden(0,true);
}

void publicWindow::sendUser(const QString &user) // 从上一个页面接收参数
{
    this->user=user;

    // 右上角显示用户昵称
    QString select=QString("select name from user where email='%1'").arg(user);
    QString name;

    query->exec(select);
    while (query->next()) {
        name=query->value(0).toString();
    }

    ui->creatorBt->setText(name);
    ui->userBt->setText(name);

    // 显示今天的发起待办
    QDate today=QDate::currentDate();
    QString str=QString("select * from publicTodo where date='%1'and creator='%2'").arg(today.toString("yyyy-MM-dd")).arg(user);
    setCreateTV(str);

    // 显示今天的参与待办，发起的待办不显示在参与待办中
    str=QString("select publicTodo.* from publicTodo,creator_user where id=tid and date='%1' and memberEmail='%2' and creator!='%2'").arg(today.toString("yyyy-MM-dd")).arg(user);
    setJoinTV(str);

    // 设置发起待办的标签下拉框
    select=QString("select distinct(tag) from publicTodo where creator='%1'").arg(user);
    query->exec(select);
    while (query->next()) {
        QString tag=query->value(0).toString();
        ui->searchCB->addItem(tag);
    }

    // 设置参与待办的标签下拉框
    select=QString("select distinct(tag) from publicTodo,creator_user where id=tid and memberEmail='%1'").arg(user);
    query->exec(select);
    while (query->next()) {
        QString tag=query->value(0).toString();
        ui->joinSearchCB->addItem(tag);
    }
}

void publicWindow::on_createCW_selectionChanged() // 点击日历，显示选中日期的发起待办
{
    QDate selectDate=ui->createCW->selectedDate();
    QString str=QString("select * from publicTodo where date='%1'and creator='%2'").arg(selectDate.toString("yyyy-MM-dd")).arg(user);
    setCreateTV(str);
}

void publicWindow::setSearchCB() // 设置标签下拉框
{
    ui->searchCB->clear(); // 清空之前的数据

    QString select=QString("select distinct(tag) from publicTodo where creator='%1'").arg(user);
    query->exec(select);
    while (query->next()) {
        QString tag=query->value(0).toString();
        ui->searchCB->addItem(tag);
    }
}

void publicWindow::on_insertBt_clicked() // 弹出插入窗口
{
    QDate selectDate=ui->createCW->selectedDate();
    publicInsert *insertDia=new publicInsert;

    insertDia->setModal(true); // 窗口未关闭时不能操作其他页面
    insertDia->show();
    insertDia->sendInfo(selectDate,user,this);
}

void publicWindow::onCreateTVClicked()
{
    ui->createTV->setSelectionBehavior(QAbstractItemView::SelectRows); // 点击单元格选中整行

    // 获取选中行的数据
    int currentRow = ui->createTV->currentIndex().row();
    QAbstractItemModel *modelTV = ui->createTV->model ();
    QModelIndex idIndex = modelTV->index(currentRow,5);
    QVariant idData = modelTV->data(idIndex);
    int selectId=idData.toInt();
    QString select=QString("select * from creator_user where tid='%1'").arg(selectId);

    // 显示选中待办的参与成员
    QSqlQueryModel *model=new QSqlQueryModel(ui->memberTV);
    model->setQuery(select,this->db);
    model->setHeaderData(1,Qt::Horizontal,tr("成员昵称"));
    model->setHeaderData(2,Qt::Horizontal,tr("成员邮箱"));
    ui->memberTV->setModel(model);
    ui->memberTV->setColumnHidden(0,true);
}

void publicWindow::onJoinTVClicked()
{
    ui->joinTV->setSelectionBehavior(QAbstractItemView::SelectRows); // 点击单元格选中整行

    // 获取选中行的数据
    int currentRow = ui->joinTV->currentIndex().row();
    QAbstractItemModel *modelTV = ui->joinTV->model ();
    QModelIndex idIndex = modelTV->index(currentRow,5);
    QVariant idData = modelTV->data(idIndex);
    int selectId=idData.toInt();
    QString select=QString("select * from creator_user where tid='%1'").arg(selectId);

    // 显示选中待办的参与成员
    QSqlQueryModel *model=new QSqlQueryModel(ui->joinMemberTV);
    model->setQuery(select,this->db);
    model->setHeaderData(1,Qt::Horizontal,tr("成员昵称"));
    model->setHeaderData(2,Qt::Horizontal,tr("成员邮箱"));
    ui->joinMemberTV->setModel(model);
    ui->joinMemberTV->setColumnHidden(0,true);
}

void publicWindow::on_deleteOneBt_clicked() // 仅删除当前待办
{
    int ok = QMessageBox::warning(this,tr("删除当前待办!"),tr("确认删除当前待办？"),QMessageBox::Yes,QMessageBox::No);

    if(ok == QMessageBox::Yes)
    {
        // 获取选中待办的数据
        int currentRow = ui->createTV->currentIndex().row();
        QAbstractItemModel *modelTV = ui->createTV->model ();
        QModelIndex idIndex = modelTV->index(currentRow,5);
        QModelIndex dateIndex=modelTV->index(currentRow,2);
        QVariant idData = modelTV->data(idIndex);
        QVariant dateData=modelTV->data(dateIndex);
        int selectId=idData.toInt();
        QDate selectDate=dateData.toDate();

        // 把最大的id替换为被删除的id
        QString select=QString("select max(tid) from creator_user");
        int id=0;

        query->exec(select);
        while (query->next()) {
            id=query->value(0).toInt();
        }

        QString deleteStr=QString("delete from publicTodo where id='%1'and date='%2'").arg(selectId).arg(selectDate.toString("yyyy-MM-dd"));
        query->exec(deleteStr);

        QString deleteOne=QString("delete from creator_user where tid='%1'").arg(selectId);
        query->exec(deleteOne);

        QString update=QString("update publicTodo set id='%1' where id='%2'").arg(selectId).arg(id);
        query->exec(update);
        update=QString("update creator_user set tid='%1' where tid='%2'").arg(selectId).arg(id);
        query->exec(update);

        QSqlQueryModel *nullModel=new QSqlQueryModel(ui->memberTV);
        ui->memberTV->setModel(nullModel);

        // 删除后显示所有发起待办
        on_createSearchAllBt_clicked();
    }
}

void publicWindow::on_deleteAllBt_clicked() // 删除所有重复待办
{
    int ok = QMessageBox::warning(this,tr("删除所有重复待办!"),tr("确认删除所有重复待办？"),QMessageBox::Yes,QMessageBox::No);

    if(ok == QMessageBox::Yes)
    {
        // 获取选中待办的数据
        int currentRow = ui->createTV->currentIndex().row();
        QAbstractItemModel *modelTV = ui->createTV->model ();
        QModelIndex idIndex = modelTV->index(currentRow,5);
        QVariant idData = modelTV->data(idIndex);
        int selectId=idData.toInt();
        QString deleteStr=QString("delete from publicTodo where id='%1'").arg(selectId);

        // 把最大的id替换成被删除的id
        QString select=QString("select max(tid) from creator_user");
        int id=0;

        query->exec(select);
        while (query->next()) {
            id=query->value(0).toInt();
        }
        query->exec(deleteStr);

        QString deleteAll=QString("delete from creator_user where tid='%1'").arg(selectId);
        query->exec(deleteAll);

        QString update=QString("update publicTodo set id='%1' where id='%2'").arg(selectId).arg(id);
        query->exec(update);
        update=QString("update creator_user set tid='%1' where tid='%2'").arg(selectId).arg(id);
        query->exec(update);

        // 删除后显示全部发起待办
        on_createSearchAllBt_clicked();
    }
}

void publicWindow::on_createSearchBt_clicked() // 按标签查找发起待办
{
    QString selectTag=ui->searchCB->currentText();
    QDate today=QDate::currentDate();
    QString str=QString("select * from publicTodo where tag='%1'and creator='%2'and date>='%3'").arg(selectTag).arg(user).arg(today.toString("yyyy-MM-dd"));
    setCreateTV(str);
}

void publicWindow::on_createSearchAllBt_clicked() // 查看全部发起待办
{
    QDate today=QDate::currentDate();
    QString str=QString("select * from publicTodo where creator='%1'and date>='%2'").arg(user).arg(today.toString("yyyy-MM-dd"));
    setCreateTV(str);
}

void publicWindow::on_joinCW_selectionChanged() // 点击日历，显示选中日期的参与待办
{
    QDate selectDate=ui->joinCW->selectedDate();
    QString str=QString("select publicTodo.* from publicTodo,creator_user where id=tid and date='%1'and memberEmail='%2'").arg(selectDate.toString("yyyy-MM-dd")).arg(user);
    setJoinTV(str);
}

void publicWindow::on_joinSearchBt_clicked() // 按标签查找参与待办
{
    QDate today=QDate::currentDate();
    QString selectTag=ui->joinSearchCB->currentText();
    QString str=QString("select publicTodo.* from publicTodo,creator_user where id=tid and tag='%1' and memberEmail='%2' and creator!='%2'and date>='%3'").arg(selectTag).arg(user).arg(today.toString("yyyy-MM-dd"));
    setJoinTV(str);
}

void publicWindow::on_joinSearchAllBt_clicked() // 查看全部参与待办
{
    QDate today=QDate::currentDate();
    QString str=QString("select publicTodo.* from publicTodo,creator_user where id=tid and memberEmail='%1' and creator!='%1'and date>='%2'").arg(user).arg(today.toString("yyyy-MM-dd"));
    setJoinTV(str);
}

void publicWindow::showUser() // 返回个人主页，关闭当前页面
{
    SelectWindow *selectWin=new SelectWindow;
    selectWin->show();
    selectWin->sendUser(user);
    this->close();
}

void publicWindow::on_creatorBt_clicked()
{
    showUser();
}

void publicWindow::on_userBt_clicked()
{
    showUser();
}

void publicWindow::on_createTV_doubleClicked(const QModelIndex &index) // 双击待办显示具体内容
{
    ui->createTV->setSelectionBehavior(QAbstractItemView::SelectRows); // 点击单元格选中整行

    // 获取选中行的数据
    int currentRow = ui->createTV->currentIndex().row();
    QAbstractItemModel *modelTV = ui->createTV->model ();
    QModelIndex idIndex = modelTV->index(currentRow,5);
    QModelIndex dateIndex=modelTV->index(currentRow,2);
    QVariant idData = modelTV->data(idIndex);
    QVariant dateData=modelTV->data(dateIndex);
    int selectId=idData.toInt();
    QDate selectDate=dateData.toDate();

    // 弹出新窗口
    publicUpdate *publicUpdateDia=new publicUpdate;
    publicUpdateDia->setModal(true); // 窗口关闭之前不能操作其他页面
    publicUpdateDia->show();
    publicUpdateDia->sendInfo(user,this,selectId,selectDate);
}

void publicWindow::on_joinTV_doubleClicked(const QModelIndex &index) // 双击待办显示具体内容
{
    ui->joinTV->setSelectionBehavior(QAbstractItemView::SelectRows); // 点击单元格选中整行

    // 获取选中行的内容
    int currentRow = ui->joinTV->currentIndex().row();
    QAbstractItemModel *modelTV = ui->joinTV->model ();
    QModelIndex idIndex = modelTV->index(currentRow,5);
    QModelIndex dateIndex=modelTV->index(currentRow,2);
    QVariant idData = modelTV->data(idIndex);
    QVariant dateData=modelTV->data(dateIndex);
    int selectId=idData.toInt();
    QDate selectDate=dateData.toDate();

    // 弹出新窗口
    publicShow *publicShowDia=new publicShow;
    publicShowDia->setModal(true); // 关闭窗口前不能操作其他页面
    publicShowDia->show();
    publicShowDia->sendInfo(user,selectId,selectDate);
}

void publicWindow::on_joinUpdateNameBt_clicked() // 参与待办者可以修改自己在该待办中的昵称
{
    bool ok;
    QString updateName=QInputDialog::getText(this,tr("修改昵称"),tr("请输入修改后的昵称"),QLineEdit::Normal,0,&ok);

    if(ok&&!updateName.isEmpty())
    {
        // 获取选中待办的数据
        int currentRow = ui->joinTV->currentIndex().row();
        QAbstractItemModel *modelTV = ui->joinTV->model ();
        QModelIndex idIndex = modelTV->index(currentRow,5);
        QVariant idData = modelTV->data(idIndex);
        int selectId=idData.toInt();

        // 更新数据库中的数据
        QString update=QString("update creator_user set memberName='%1' where tid='%2'and memberEmail='%3'").arg(updateName).arg(selectId).arg(user);
        QString select=QString("select * from creator_user where tid='%1'").arg(selectId);
        query->exec(update);

        // 重置参与成员的tableview
        QSqlQueryModel *model=new QSqlQueryModel(ui->joinMemberTV);
        model->setQuery(select,this->db);
        model->setHeaderData(1,Qt::Horizontal,tr("成员昵称"));
        model->setHeaderData(2,Qt::Horizontal,tr("成员邮箱"));
        ui->joinMemberTV->setModel(model);
        ui->joinMemberTV->setColumnHidden(0,true);
    }
}
