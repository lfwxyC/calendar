#include "publicshow.h"
#include "ui_publicshow.h"

#include <QtSql/QSqlQuery>
#include <QSqlTableModel>
#include <QInputDialog>

publicShow::publicShow(QWidget *parent) : // 显示待办
    QDialog(parent),
    ui(new Ui::publicShow)
{
    ui->setupUi(this);

    // 设置数据库
    db=QSqlDatabase::database("conn");
    query=new QSqlQuery(db);
}

publicShow::~publicShow()
{
    delete ui;
    delete query;
}

void publicShow::sendInfo(const QString &user, int id, QDate &date) // 从上一个页面接收参数
{
    this->user=user;
    this->id=id;

    // 设置选中待办的数据
    QString select=QString("select * from publicTodo where id='%1'and date='%2'").arg(id).arg(date.toString("yyyy-MM-dd"));
    QString title,text,currentTag,duplicate;
    QDate today=QDate::currentDate();
    QTime time;

    query->exec(select);
    while (query->next()) {
        currentTag=query->value(0).toString();
        title=query->value(1).toString();
        time=query->value(3).toTime();
        text=query->value(6).toString();
        duplicate=query->value(7).toString();
    }

    // 只读，不能编辑
    ui->titleLineEdit->setText(title);
    ui->titleLineEdit->setReadOnly(true);
    ui->comboBox->addItem(currentTag);
    ui->comboBox->setEnabled(false);
    ui->dateEdit->setMinimumDate(today);
    ui->dateEdit->setDate(date);
    ui->dateEdit->setCalendarPopup(true);
    ui->dateEdit->setEnabled(false);
    ui->timeEdit->setTime(time);
    ui->timeEdit->setEnabled(false);
    ui->textEdit->setText(text);
    ui->textEdit->setReadOnly(true);
    ui->duplicateLine->setText(duplicate);
    ui->duplicateLine->setReadOnly(true);

    // 显示选中待办的参与成员
    select=QString("select * from creator_user where tid='%1'").arg(id);

    QSqlQueryModel *model=new QSqlQueryModel(ui->memberTV);
    model->setQuery(select,this->db);
    model->setHeaderData(1,Qt::Horizontal,tr("成员昵称"));
    model->setHeaderData(2,Qt::Horizontal,tr("成员邮箱"));
    ui->memberTV->setModel(model);
    ui->memberTV->setColumnHidden(0,true);
}
