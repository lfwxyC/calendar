#include "publicupdate.h"
#include "ui_publicupdate.h"
#include "readonlydelegate.h"

#include <QInputDialog>
#include <QtSql/QSqlQuery>
#include <QMessageBox>
#include <QSqlTableModel>
#include <QMenu>

publicUpdate::publicUpdate(QWidget *parent) : // 更新发起待办
    QDialog(parent),
    ui(new Ui::publicUpdate)
{
    ui->setupUi(this);

    // 设置数据库
    db=QSqlDatabase::database("conn");
    query=new QSqlQuery(db);

    // 隐藏修改按钮
    ui->okAllBt->hide();
    ui->addMemberBt->hide();

    // 按enter键进入编辑
    ui->updateBt->setDefault(true);
}

publicUpdate::~publicUpdate()
{
    delete ui;
    delete query;
}

void publicUpdate::sendInfo(const QString &user, publicWindow *publicWin, int id, QDate &date) // 从上一个页面接收数据
{
    this->user=user;
    this->id=id;
    this->publicWin=publicWin;
    this->oldDate=date;

    // 设置选中待办的数据
    QString select=QString("select * from publicTodo where id='%1'and date='%2'").arg(id).arg(date.toString("yyyy-MM-dd"));
    QString title,text;
    QDate today=QDate::currentDate();
    QTime time;

    query->exec(select);
    while (query->next()) {
        currentTag=query->value(0).toString();
        title=query->value(1).toString();
        time=query->value(3).toTime();
        text=query->value(6).toString();
        oldDuplicate=query->value(7).toString();
    }

    // 只读，不能修改
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
    ui->duplicateCB->addItem(oldDuplicate);
    ui->duplicateCB->setEnabled(false);

    // 设置标签下拉框
    select=QString("select distinct(tag) from publicTodo where user='%1'").arg(user);

    query->exec(select);
    while (query->next()) {
        QString tag=query->value(0).toString();

        if(tag!=currentTag)
        {
            ui->comboBox->addItem(tag);
        }
    }

    // 设置重复下拉框
    if(oldDuplicate!="不重复")
    {
        ui->duplicateCB->addItem(QString::asprintf("不重复"));
    }
    if(oldDuplicate!="每周")
    {
        ui->duplicateCB->addItem(QString::asprintf("每周"));
    }
    if(oldDuplicate!="每月")
    {
        ui->duplicateCB->addItem(QString::asprintf("每月"));
    }
    if(oldDuplicate!="每年")
    {
        ui->duplicateCB->addItem(QString::asprintf("每年"));
    }

    // 显示选中待办的参与成员
    select=QString("select * from creator_user where tid='%1'").arg(id);

    QSqlQueryModel *model=new QSqlQueryModel(ui->memberTV);
    model->setQuery(select,this->db);
    model->setHeaderData(1,Qt::Horizontal,tr("成员昵称"));
    model->setHeaderData(2,Qt::Horizontal,tr("成员邮箱"));
    ui->memberTV->setModel(model);
    ui->memberTV->setColumnHidden(0,true);
}

void publicUpdate::slotContextMenu(QPoint pos)
{
   auto index = ui->memberTV->indexAt(pos);
    if (index.isValid())
    {
        popMenu->exec(QCursor::pos()); // 菜单出现的位置为当前鼠标的位置
    }
}

void publicUpdate::onDelete() // 删除待办成员
{
    // 直接在tableview对数据库进行操作
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
        model->submitAll(); //否则提交，在数据库中删除该行
        ui->memberTV->setModel(model);
        ui->memberTV->setColumnHidden(0,true);
    }
}

void publicUpdate::on_addLabelBt_clicked()
{
    bool ok;
    QString tag=QInputDialog::getText(this,tr("新增标签"),tr("请输入标签名称"),QLineEdit::Normal,0,&ok);

    if(ok&&!tag.isEmpty())
    {
        ui->comboBox->addItem(tag);
        ui->comboBox->setCurrentText(tag);
    }
}

void publicUpdate::on_updateLabelBt_clicked()
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

void publicUpdate::on_eraseLabelBt_clicked()
{
    QString deleteTag=ui->comboBox->currentText();
    int ok = QMessageBox::warning(this,tr("删除标签!"),tr("确认删除当前标签？"),QMessageBox::Yes,QMessageBox::No);

    if(ok == QMessageBox::Yes)
    {
        int index=ui->comboBox->currentIndex();
        ui->comboBox->removeItem(index);
    }
}

void publicUpdate::on_addMemberBt_clicked()
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
        else // 邮箱存在
        {
            // 在数据库中插入数据
            QString insert=QString("insert into creator_user values('%1','%2','%3')").arg(id).arg(memberName).arg(memberEmail);
            query->exec(insert);

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

            // 设置邮箱不可编辑
            ReadOnlyDelegate* readOnlyDelegate = new ReadOnlyDelegate();
            ui->memberTV->setItemDelegateForColumn(2, readOnlyDelegate);
        }
    }
}

void publicUpdate::on_okAllBt_clicked()
{
    // 获取用户输入
    QDate updateDate=ui->dateEdit->date();
    QTime updateTime=ui->timeEdit->time();
    QString updateTag=ui->comboBox->currentText();
    QString updateTitle=ui->titleLineEdit->text();
    QString updateText=ui->textEdit->toPlainText();
    QString updateDuplicate=ui->duplicateCB->currentText();
    QDate maxDate=ui->dateEdit->maximumDate();

    if(updateTitle==NULL)
    {
        QMessageBox::warning(this,"warning","内容不能为空！",QMessageBox::Ok);
    }
    else { // 内容不为空，可以更新
        if(oldDate!=updateDate||oldDuplicate!=updateDuplicate) // 修改日期或重复
        {
            // 删除原来的待办，重新插入
            QString deleteStr=QString("delete from publicTodo where id='%1'").arg(id);
            query->exec(deleteStr);

            if(updateDuplicate=="不重复")
            {
                QString insert=QString("insert into publicTodo values('%1','%2','%3','%4','%5','%6','%7','%8')").arg(updateTag).arg(updateTitle).arg(updateDate.toString("yyyy-MM-dd")).arg(updateTime.toString("hh:mm:ss")).arg(user).arg(id).arg(updateText).arg(updateDuplicate);
                query->exec(insert);
            }
            else if (updateDuplicate=="每周") {
                while (updateDate<=maxDate) {
                    QString insert=QString("insert into publicTodo values('%1','%2','%3','%4','%5','%6','%7','%8')").arg(updateTag).arg(updateTitle).arg(updateDate.toString("yyyy-MM-dd")).arg(updateTime.toString("hh:mm:ss")).arg(user).arg(id).arg(updateText).arg(updateDuplicate);
                    query->exec(insert);
                    updateDate=updateDate.addDays(7);
                }
            }
            else if (updateDuplicate=="每月") {
                while (updateDate<=maxDate) {
                    QString insert=QString("insert into publicTodo values('%1','%2','%3','%4','%5','%6','%7','%8')").arg(updateTag).arg(updateTitle).arg(updateDate.toString("yyyy-MM-dd")).arg(updateTime.toString("hh:mm:ss")).arg(user).arg(id).arg(updateText).arg(updateDuplicate);
                    query->exec(insert);
                    updateDate=updateDate.addMonths(1);
                }
            }
            else if (updateDuplicate=="每年") {
                while (updateDate<=maxDate) {
                    QString insert=QString("insert into publicTodo values('%1','%2','%3','%4','%5','%6','%7','%8')").arg(updateTag).arg(updateTitle).arg(updateDate.toString("yyyy-MM-dd")).arg(updateTime.toString("hh:mm:ss")).arg(user).arg(id).arg(updateText).arg(updateDuplicate);
                    query->exec(insert);
                    updateDate=updateDate.addYears(1);
                }
            }
        }
        else { // 不修改日期或重复，直接更新
            QString update=QString("update publicTodo set tag='%1',title='%2',time='%3',text='%4' where id='%5'").arg(updateTag).arg(updateTitle).arg(updateTime.toString("hh:mm:ss")).arg(updateText).arg(id);
            query->exec(update);
        }

        // 重置待办的tableview,关闭窗口
        updateDate=ui->dateEdit->date();
        QString str=QString("select * from publicTodo where date='%1'and creator='%2'").arg(updateDate.toString("yyyy-MM-dd")).arg(user);

        publicWin->setCreateTV(str);
        publicWin->setSearchCB();
        this->close();
    }
}

void publicUpdate::on_updateBt_clicked()
{
    // 显示修改按钮
    ui->okAllBt->show();
    ui->addMemberBt->show();
    ui->updateBt->hide();

    // 可修改
    ui->titleLineEdit->setReadOnly(false);
    ui->comboBox->setEnabled(true);
    ui->duplicateCB->setEnabled(true);
    ui->dateEdit->setEnabled(true);
    ui->timeEdit->setEnabled(true);
    ui->textEdit->setReadOnly(false);
    ui->memberTV->setContextMenuPolicy(Qt::CustomContextMenu); //可弹出右键菜单

    popMenu= new QMenu(ui->memberTV); //菜单
    popMenu->addAction("删除",this,SLOT(onDelete()));     //设置菜单项,并连接槽函数

    connect(ui->memberTV, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));

    // 设置参与成员的tableview可编辑
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

    // 设置邮箱不可编辑
    ReadOnlyDelegate* readOnlyDelegate = new ReadOnlyDelegate();
    ui->memberTV->setItemDelegateForColumn(2, readOnlyDelegate);
}
