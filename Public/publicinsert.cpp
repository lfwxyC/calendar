#include "publicinsert.h"
#include "ui_publicinsert.h"
#include "readonlydelegate.h"
#include "publicwindow.h"

#include <QStandardItemModel>
#include <QMenu>
#include <QSqlTableModel>
#include <QtSql/QSqlQuery>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>

publicInsert::publicInsert(QWidget *parent) : // 插入发起待办
    QDialog(parent),
    ui(new Ui::publicInsert)
{
    ui->setupUi(this);

    // 设置数据库
    db=QSqlDatabase::database("conn");
    query=new QSqlQuery(db);

    // 得到插入的id
    id=0;
    QString select=QString("select max(tid) from creator_user");

    query->exec(select);
    while (query->next()) {
        id=query->value(0).toInt();
    }
    id++;

    ui->memberTV->setContextMenuPolicy(Qt::CustomContextMenu); // 可弹出右键菜单
    ui->okBt->setDefault(true);

    popMenu= new QMenu(ui->memberTV); // 菜单
    popMenu->addAction("修改",this,SLOT(onUpdate()));
    popMenu->addAction("删除",this,SLOT(onDelete()));     // 设置菜单项,并连接槽函数

    connect(ui->memberTV, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));

    // 设置重复下拉框
    ui->duplicateCB->addItem(QString::asprintf("不重复"));
    ui->duplicateCB->addItem(QString::asprintf("每周"));
    ui->duplicateCB->addItem(QString::asprintf("每月"));
    ui->duplicateCB->addItem(QString::asprintf("每年"));
}

publicInsert::~publicInsert()
{
    delete ui;
    delete query;
}

void publicInsert::sendInfo(const QDate& date,const QString& user,publicWindow *publicWin) // 从上一个页面接收数据
{
    QDate today=QDate::currentDate();
    ui->dateEdit->setMinimumDate(today); // 只能选择今天之后的日期
    ui->dateEdit->setDate(date);
    ui->dateEdit->setCalendarPopup(true); // 设置日期时弹出日历

    this->user=user;
    this->publicWin=publicWin;

    // 设置标签下拉框
    QString select=QString("select distinct(tag) from publicTodo where creator='%1'").arg(user);

    query->exec(select);
    while (query->next()) {
        QString tag=query->value(0).toString();
        ui->comboBox->addItem(tag);
    }

    // 默认添加发起者至参与成员中
    select=QString("select name from user where email='%1'").arg(user);
    QString memberName;

    query->exec(select);
    while (query->next()) {
        memberName=query->value(0).toString();
    }

    QString insert=QString("insert into creator_user values('%1','%2','%3')").arg(id).arg(memberName).arg(user);
    query->exec(insert);

    // 设置参与成员的tableview
    select=QString("select * from creator_user where tid='%1'").arg(id);

    QSqlQueryModel *model=new QSqlQueryModel(ui->memberTV);
    model->setQuery(select,this->db);
    model->setHeaderData(1,Qt::Horizontal,tr("成员昵称"));
    model->setHeaderData(2,Qt::Horizontal,tr("成员邮箱"));
    ui->memberTV->setModel(model);
    ui->memberTV->setColumnHidden(0,true);
}

void publicInsert::slotContextMenu(QPoint pos)
{
   auto index = ui->memberTV->indexAt(pos);
    if (index.isValid())
    {
        popMenu->exec(QCursor::pos()); // 菜单出现的位置为当前鼠标的位置
    }
}

void publicInsert::onDelete() // 删除参与成员
{
    // 直接从tableview对数据库进行操作
    int currentRow = ui->memberTV->currentIndex().row();
    QSqlTableModel *model=new QSqlTableModel(this,db);
    QString str=QString("tid='%1'").arg(id);

    model->setTable("creator_user");
    model->setFilter(str);
    model->select();
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->setHeaderData(1,Qt::Horizontal,tr("成员昵称"));
    model->setHeaderData(2,Qt::Horizontal,tr("成员邮箱"));
    ui->memberTV->setModel(model);
    ui->memberTV->setColumnHidden(0,true);

    //删除该行
    int ok = QMessageBox::warning(this,tr("删除"),tr("确定删除当前成员？"),QMessageBox::Yes,QMessageBox::No);
    if(ok == QMessageBox::Yes)
    {
        model->removeRow(currentRow);
        model->submitAll(); //提交，在数据库中删除该行
        ui->memberTV->setModel(model);
        ui->memberTV->setColumnHidden(0,true);
    }
}

void publicInsert::onUpdate() // 修改成员昵称
{
    // 直接从tableview对数据库进行操作
    QSqlTableModel *model=new QSqlTableModel(this,db);
    QString str=QString("tid='%1'").arg(id);

    model->setTable("creator_user");
    model->setFilter(str);
    model->select();
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->setHeaderData(1,Qt::Horizontal,tr("成员昵称"));
    model->setHeaderData(2,Qt::Horizontal,tr("成员邮箱"));
    ui->memberTV->setModel(model);
    ui->memberTV->setColumnHidden(0,true);

    // 将成员邮箱设置为不可编辑
    ReadOnlyDelegate* readOnlyDelegate = new ReadOnlyDelegate();
    ui->memberTV->setItemDelegateForColumn(2, readOnlyDelegate);
}

void publicInsert::on_addMemberBt_clicked()
{
    bool ok;
    QString memberEmail=QInputDialog::getText(this,tr("添加成员"),tr("请输入成员邮箱"),QLineEdit::Normal,0,&ok);

    if(ok&&!memberEmail.isEmpty())
    {
        QString select=QString("select name from user where email='%1'").arg(memberEmail);
        QString memberName;

        query->exec(select);
        while (query->next()) {
            memberName=query->value(0).toString();
        }
        if(memberName==NULL)
        {
            QMessageBox::warning(this,"warning","邮箱不存在！",QMessageBox::Ok);
        }
        else { // 邮箱存在
            // 将成员插入到数据库中
            QString insert=QString("insert into creator_user values('%1','%2','%3')").arg(id).arg(memberName).arg(memberEmail);
            query->exec(insert);

            // 重置参与成员的tableview
            select=QString("select * from creator_user where tid='%1'").arg(id);

            QSqlQueryModel *model=new QSqlQueryModel(ui->memberTV);
            model->setQuery(select,this->db);
            model->setHeaderData(1,Qt::Horizontal,tr("成员昵称"));
            model->setHeaderData(2,Qt::Horizontal,tr("成员邮箱"));
            ui->memberTV->setModel(model);
            ui->memberTV->setColumnHidden(0,true);
        }
    }
}

void publicInsert::on_addLabelBt_clicked()
{
    bool ok;
    QString tag=QInputDialog::getText(this,tr("新增标签"),tr("请输入标签名称"),QLineEdit::Normal,0,&ok);

    if(ok&&!tag.isEmpty())
    {
        QString selectTag;
        QString select=QString("select * from publicTodoTag where user='%1' and tag='%2'").arg(user).arg(tag);

        query->exec(select);
        while (query->next()) {
            selectTag=query->value(0).toString();
            if(tag==selectTag)
            {
                QMessageBox::warning(this,"warning","标签已存在！",QMessageBox::Ok);
                return;
            }
        }

        ui->comboBox->addItem(tag);
        ui->comboBox->setCurrentText(tag);
    }
}

void publicInsert::on_updateLabelBt_clicked()
{
    QString currentTag=ui->comboBox->currentText();
    int index=ui->comboBox->currentIndex();
    bool ok;
    QString updateTag=QInputDialog::getText(this,tr("修改标签"),tr("请输入修改后的标签名称"),QLineEdit::Normal,0,&ok);

    if(ok&&!updateTag.isEmpty())
    {
        ui->comboBox->setEditable(true);
        ui->comboBox->setItemText(index,updateTag);
    }
}

void publicInsert::on_eraseLabelBt_clicked()
{
    QString deleteTag=ui->comboBox->currentText();
    int ok = QMessageBox::warning(this,tr("删除标签!"),tr("确认删除当前标签？"),QMessageBox::Yes,QMessageBox::No);

    if(ok == QMessageBox::Yes)
    {
        int index=ui->comboBox->currentIndex();        
        ui->comboBox->removeItem(index);
    }
}

void publicInsert::on_okBt_clicked()
{
    // 获取用户输入
    QDate insertDate=ui->dateEdit->date();
    QTime insertTime=ui->timeEdit->time();
    QString insertTag=ui->comboBox->currentText();
    QString insertTitle=ui->titleLineEdit->text();
    QString insertText=ui->textEdit->toPlainText();
    QString duplicate=ui->duplicateCB->currentText();
    QDate maxDate=ui->dateEdit->maximumDate();

    if(insertTitle==NULL)
    {
        QMessageBox::warning(this,"warning","主题不能为空！",QMessageBox::Ok);
    }
    else { // 主题不为空，可以插入
        if(duplicate=="不重复")
        {
            QString insert=QString("insert into publicTodo values('%1','%2','%3','%4','%5','%6','%7','%8')").arg(insertTag).arg(insertTitle).arg(insertDate.toString("yyyy-MM-dd")).arg(insertTime.toString("hh:mm:ss")).arg(user).arg(id).arg(insertText).arg(duplicate);
            query->exec(insert);
        }
        else if (duplicate=="每周") {
            while (insertDate<=maxDate) {
                QString insert=QString("insert into publicTodo values('%1','%2','%3','%4','%5','%6','%7','%8')").arg(insertTag).arg(insertTitle).arg(insertDate.toString("yyyy-MM-dd")).arg(insertTime.toString("hh:mm:ss")).arg(user).arg(id).arg(insertText).arg(duplicate);
                query->exec(insert);
                insertDate=insertDate.addDays(7);
            }
        }
        else if (duplicate=="每月") {
            while (insertDate<=maxDate) {
                QString insert=QString("insert into publicTodo values('%1','%2','%3','%4','%5','%6','%7','%8')").arg(insertTag).arg(insertTitle).arg(insertDate.toString("yyyy-MM-dd")).arg(insertTime.toString("hh:mm:ss")).arg(user).arg(id).arg(insertText).arg(duplicate);
                query->exec(insert);
                insertDate=insertDate.addMonths(1);
            }
        }
        else if (duplicate=="每年") {
            while (insertDate<=maxDate) {
                QString insert=QString("insert into publicTodo values('%1','%2','%3','%4','%5','%6','%7','%8')").arg(insertTag).arg(insertTitle).arg(insertDate.toString("yyyy-MM-dd")).arg(insertTime.toString("hh:mm:ss")).arg(user).arg(id).arg(insertText).arg(duplicate);
                query->exec(insert);
                insertDate=insertDate.addYears(1);
            }
        }

        // 关闭窗口，显示插入日期的所有待办
        insertDate=ui->dateEdit->date();
        QString str=QString("select * from publicTodo where date='%1'and creator='%2'").arg(insertDate.toString("yyyy-MM-dd")).arg(user);
        publicWin->setCreateTV(str);
        publicWin->setSearchCB();
        this->close();
    }
}

void publicInsert::on_cancelBt_clicked()
{
    QString deleteStr=QString("delete from creator_user where tid='%1'").arg(id);

    query->exec(deleteStr);
    publicWin->setSearchCB();
    this->close();
}
