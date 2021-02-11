#include "privatewindow.h"
#include "ui_privatewindow.h"
#include "selectwindow.h"
#include "recordtext.h"
#include "recordinsert.h"
#include "privatetodoinsert.h"
#include "privatetodoupdate.h"

#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlQuery>
#include <QInputDialog>
#include <QSqlTableModel>
#include <QMessageBox>
#include <QDebug>

PrivateWindow::PrivateWindow(QWidget *parent) : // 私人页面
    QMainWindow(parent),
    ui(new Ui::PrivateWindow)
{
    ui->setupUi(this);

    // 设置数据库
    db=QSqlDatabase::database("conn");
    query=new QSqlQuery(db);

    QDate today=QDate::currentDate();

    // 设置记录日历样式
    ui->recordCalendarWidget->setMaximumDate(today); // 只能选择今天之前的日期
    ui->recordCalendarWidget->setWeekdayTextFormat(Qt::Saturday,ui->recordCalendarWidget->weekdayTextFormat(Qt::Monday));
    ui->recordCalendarWidget->setWeekdayTextFormat(Qt::Sunday,ui->recordCalendarWidget->weekdayTextFormat(Qt::Monday));
    ui->recordCalendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);

    // 设置待办日历样式
    ui->todoCW->setMinimumDate(today); // 只能选择今天之后的日期
    ui->todoCW->setWeekdayTextFormat(Qt::Saturday,ui->todoCW->weekdayTextFormat(Qt::Monday));
    ui->todoCW->setWeekdayTextFormat(Qt::Sunday,ui->todoCW->weekdayTextFormat(Qt::Monday));
    ui->todoCW->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
}

PrivateWindow::~PrivateWindow()
{
    delete ui;
    delete query;
}

void PrivateWindow::setRecordTV(const QString& str) // 设置记录的tableview
{
    QSqlQueryModel *model=new QSqlQueryModel(ui->recordTV);

    model->setQuery(str,this->db);
    model->setHeaderData(0,Qt::Horizontal,tr("标签"));
    model->setHeaderData(1,Qt::Horizontal,tr("内容"));
    model->setHeaderData(2,Qt::Horizontal,tr("日期"));

    ui->recordTV->setModel(model);
    ui->recordTV->setColumnHidden(3,true);
    ui->recordTV->setColumnHidden(4,true);
}

void PrivateWindow::setTodoTV(const QString& str) // 设置待办的tableview
{
    QSqlQueryModel *model=new QSqlQueryModel(ui->todoTV);

    model->setQuery(str,this->db);
    model->setHeaderData(0,Qt::Horizontal,tr("标签"));
    model->setHeaderData(1,Qt::Horizontal,tr("主题"));
    model->setHeaderData(2,Qt::Horizontal,tr("日期"));
    model->setHeaderData(3,Qt::Horizontal,tr("时间"));
    model->setHeaderData(6,Qt::Horizontal,tr("重复"));

    ui->todoTV->setModel(model);
    ui->todoTV->setColumnHidden(4,true);
    ui->todoTV->setColumnHidden(5,true);
}

void PrivateWindow::setSearchCB() // 设置记录的combobox
{
    ui->searchCB->clear();
    QString select=QString("select distinct(rtag) from privateRecord where ruser='%1'").arg(user); // 从数据库中查找标签

    query->exec(select);
    while (query->next()) {
        QString tag=query->value(0).toString();
        ui->searchCB->addItem(tag);
    }
}

void PrivateWindow::setTodoSearchCB() // 设置待办的combobox
{
    ui->todoSearchCB->clear();
    QString select=QString("select distinct(ttag) from privateTodo where tuser='%1'").arg(user); // 从数据库中查找标签

    query->exec(select);
    while (query->next()) {
        QString tag=query->value(0).toString();
        ui->todoSearchCB->addItem(tag);
    }
}

int PrivateWindow::getRecordId() // 获取记录id
{
    int currentRow = ui->recordTV->currentIndex().row();
    QAbstractItemModel *modelTV = ui->recordTV->model ();
    QModelIndex idIndex = modelTV->index(currentRow,4);
    QVariant idData = modelTV->data(idIndex);
    int selectId=idData.toInt();
    return selectId;
}

/*void PrivateWindow::getTodoInfo(int &selectId, QDate &selectDate)
{
    int currentRow = ui->todoTV->currentIndex().row();
    QAbstractItemModel *modelTV = ui->todoTV->model ();
    QModelIndex idIndex = modelTV->index(currentRow,5);
    QModelIndex dateIndex=modelTV->index(currentRow,2);
    QVariant idData = modelTV->data(idIndex);
    QVariant dateData=modelTV->data(dateIndex);
    selectId=idData.toInt();
    selectDate=dateData.toDate();
}*/

void PrivateWindow::on_recordCalendarWidget_selectionChanged() // 点击日历，显示相应日期的记录
{
    QDate selectDate=ui->recordCalendarWidget->selectedDate();
    QString str=QString("select * from privateRecord where rdate='%1'and ruser='%2'").arg(selectDate.toString("yyyy-MM-dd")).arg(user);
    setRecordTV(str);
}

void PrivateWindow::on_insertBt_clicked() // 插入新记录，弹出插入窗口
{
    QDate selectDate=ui->recordCalendarWidget->selectedDate();
    recordInsert *insertDia=new recordInsert;

    insertDia->setModal(true);
    insertDia->show();
    insertDia->sendInfo(selectDate,user,this);
}

void PrivateWindow::on_deleteBt_clicked() // 删除记录
{
    int ok = QMessageBox::warning(this,tr("删除当前行!"),tr("确认删除当前行？"),QMessageBox::Yes,QMessageBox::No);

    if(ok == QMessageBox::Yes)
    {      
        // 把最大的id替换成被删除的id
        int selectId=getRecordId();
        int id=0;
        QString select=QString("select max(id) from privateRecord");       

        query->exec(select);
        while (query->next()) {
            id=query->value(0).toInt();
        }

        QString deleteStr=QString("delete from privateRecord where id='%1'").arg(selectId);
        query->exec(deleteStr);

        QString update=QString("update privateRecord set id='%1' where id='%2'").arg(selectId).arg(id);
        query->exec(update);

        on_selectAllBt_clicked(); //删除之后显示全部记录
    }
}

void PrivateWindow::on_selectBt_clicked() // 按标签查找记录
{
    QString selectTag=ui->searchCB->currentText();
    QString str=QString("select * from privateRecord where rtag='%1'and ruser='%2'").arg(selectTag).arg(user);
    setRecordTV(str);
}

void PrivateWindow::on_selectAllBt_clicked() // 查看全部记录
{
    QString str=QString("select * from privateRecord where ruser='%1'").arg(user);
    setRecordTV(str);
}

void PrivateWindow::on_todoCW_selectionChanged() // 点击日历，显示相应日期的待办
{
    QDate selectDate=ui->todoCW->selectedDate();
    QString str=QString("select * from privateTodo where tdate='%1'and tuser='%2'").arg(selectDate.toString("yyyy-MM-dd")).arg(user);
    setTodoTV(str);
}

void PrivateWindow::on_todoInsertBt_clicked() // 插入待办，弹出插入窗口
{
    QDate selectDate=ui->todoCW->selectedDate();
    privateTodoInsert *insertDia=new privateTodoInsert;

    insertDia->setModal(true);
    insertDia->show();
    insertDia->sendInfo(selectDate,user,this);
}

void PrivateWindow::on_todoUpdateBt_clicked() // 更新待办
{
    // 获取选中行的数据
    int currentRow = ui->todoTV->currentIndex().row();
    QAbstractItemModel *modelTV = ui->todoTV->model ();
    QModelIndex idIndex = modelTV->index(currentRow,5);
    QModelIndex dateIndex=modelTV->index(currentRow,2);
    QVariant idData = modelTV->data(idIndex);
    QVariant dateData=modelTV->data(dateIndex);
    int selectId=idData.toInt();
    QDate selectDate=dateData.toDate();

    // 弹出更新窗口
    privateTodoUpdate *privateTodoUpdateDia=new privateTodoUpdate;
    privateTodoUpdateDia->setModal(true);
    privateTodoUpdateDia->show();
    privateTodoUpdateDia->sendInfo(user,this,selectId,selectDate);
}

void PrivateWindow::on_deleteOneBt_clicked() // 仅删除当前待办
{
    int ok = QMessageBox::warning(this,tr("删除当前待办!"),tr("确认删除当前待办？"),QMessageBox::Yes,QMessageBox::No);

    if(ok == QMessageBox::Yes)
    {
        // 获取选中行的数据
        int currentRow = ui->todoTV->currentIndex().row();
        QAbstractItemModel *modelTV = ui->todoTV->model ();
        QModelIndex idIndex = modelTV->index(currentRow,5);
        QModelIndex dateIndex=modelTV->index(currentRow,2);
        QVariant idData = modelTV->data(idIndex);
        QVariant dateData=modelTV->data(dateIndex);
        int selectId=idData.toInt();
        QDate selectDate=dateData.toDate();       

        // 将最大的id替换为被删除的id
        QString deleteStr=QString("delete from privateTodo where id='%1'and tdate='%2'").arg(selectId).arg(selectDate.toString("yyyy-MM-dd"));
        QString select=QString("select max(id) from privateTodo");      
        int id=0;

        query->exec(select);
        while (query->next()) {
            id=query->value(0).toInt();
        }
        query->exec(deleteStr);

        QString update=QString("update privateTodo set id='%1' where id='%2'").arg(selectId).arg(id);
        query->exec(update);

        // 删除后显示全部待办
        on_todoSelectAllBt_clicked();
    }
}

void PrivateWindow::on_deleteAllBt_clicked() // 删除所有重复待办
{
    int ok = QMessageBox::warning(this,tr("删除所有重复待办!"),tr("确认删除所有重复待办？"),QMessageBox::Yes,QMessageBox::No);

    if(ok == QMessageBox::Yes)
    {
        // 获取选中行数据
        int currentRow = ui->todoTV->currentIndex().row();
        QAbstractItemModel *modelTV = ui->todoTV->model ();
        QModelIndex idIndex = modelTV->index(currentRow,5);
        QVariant idData = modelTV->data(idIndex);
        int selectId=idData.toInt();

        // 将最大的id替换为被删除的id
        QString deleteStr=QString("delete from privateTodo where id='%1'").arg(selectId);
        QString select=QString("select max(id) from privateTodo");
        int id=0;

        query->exec(select);
        while (query->next()) {
            id=query->value(0).toInt();
        }
        query->exec(deleteStr);

        QString update=QString("update privateTodo set id='%1' where id='%2'").arg(selectId).arg(id);
        query->exec(update);

        // 删除后显示所有待办
        on_todoSelectAllBt_clicked();
    }
}

void PrivateWindow::on_todoSelectBt_clicked() // 按标签查找待办
{
    QString selectTag=ui->todoSearchCB->currentText();
    QDate today=QDate::currentDate();
    QString str=QString("select * from privateTodo where ttag='%1'and tuser='%2'and tdate>='%3'").arg(selectTag).arg(user).arg(today.toString("yyyy-MM-dd"));
    setTodoTV(str);
}

void PrivateWindow::on_todoSelectAllBt_clicked() // 查看全部待办
{
    QDate today=QDate::currentDate();
    QString str=QString("select * from privateTodo where tuser='%1'and tdate>='%2'").arg(user).arg(today.toString("yyyy-MM-dd"));
    setTodoTV(str);
}

void PrivateWindow::sendUser(const QString &user) // 从上一个页面接收参数
{
    this->user=user;

    // 右上角显示用户昵称
    QString select=QString("select name from user where email='%1'").arg(user);
    QString name;

    query->exec(select);
    while (query->next()) {
        name=query->value(0).toString();
    }

    ui->recordUserBt->setText(name);
    ui->userBt->setText(name);

    // 显示今天的记录
    QDate today=QDate::currentDate();
    QString str=QString("select * from privateRecord where rdate='%1'and ruser='%2'").arg(today.toString("yyyy-MM-dd")).arg(user);
    setRecordTV(str);

    // 设置记录的标签下拉框
    select=QString("select distinct(rtag) from privateRecord where ruser='%1'").arg(user);

    query->exec(select);
    while (query->next()) {
        QString tag=query->value(0).toString();
        ui->searchCB->addItem(tag);
    }

    // 显示今天的待办
    str=QString("select * from privateTodo where tdate='%1'and tuser='%2'").arg(today.toString("yyyy-MM-dd")).arg(user);
    setTodoTV(str);

    // 设置待办的标签下拉框
    select=QString("select distinct(ttag) from privateTodo where tuser='%1'").arg(user);

    query->exec(select);
    while (query->next()) {
        QString tag=query->value(0).toString();
        ui->todoSearchCB->addItem(tag);
    }
}

void PrivateWindow::showUser() // 显示个人主页，关闭当前页面
{
    SelectWindow *selectWin=new SelectWindow;
    selectWin->show();
    selectWin->sendUser(user);
    this->close();
}

void PrivateWindow::on_userBt_clicked()
{
    showUser();
}

void PrivateWindow::on_recordUserBt_clicked()
{
    showUser();
}

void PrivateWindow::on_recordTV_doubleClicked(const QModelIndex &index) // 双击记录，显示具体内容
{
    ui->recordTV->setSelectionBehavior(QAbstractItemView::SelectRows); // 点击单元格选中一行
    int selectId=getRecordId();

    // 弹出窗口
    recordText *recordDia=new recordText;
    recordDia->setModal(true);
    recordDia->show();
    recordDia->sendInfo(user,this,selectId);
}
