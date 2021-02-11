#include "recordinsert.h"
#include "ui_recordinsert.h"
#include "privatewindow.h"

#include <QtSql/QSqlQuery>
#include <QInputDialog>
#include <QMessageBox>
#include <QtSql/QSqlQueryModel>

recordInsert::recordInsert(QWidget *parent) : // 插入记录
    QDialog(parent),
    ui(new Ui::recordInsert)
{
    ui->setupUi(this);

    // 设置数据库
    db=QSqlDatabase::database("conn");
    query=new QSqlQuery(db);  
}

recordInsert::~recordInsert()
{
    delete ui;
    delete query;
}

void recordInsert::sendInfo(const QDate& date,const QString& user,PrivateWindow *privateWin) // 从上一个页面接收参数
{
    QDate today=QDate::currentDate();


    ui->dateEdit->setMaximumDate(today); // 只能选择今天之前的日期
    ui->dateEdit->setDate(date); // 显示日历选中的日期
    ui->dateEdit->setCalendarPopup(true); // 选择日期时弹出日历

    this->user=user;
    this->privateWin=privateWin;

    // 设置标签下拉框
    QString select=QString("select distinct(rtag) from privateRecord where ruser='%1'").arg(user);
    query->exec(select);
    while (query->next()) {
        QString tag=query->value(0).toString();
        ui->comboBox->addItem(tag);
    }
}

void recordInsert::on_addLabelBt_clicked()
{
    bool ok;
    QString tag=QInputDialog::getText(this,tr("新增标签"),tr("请输入标签名称"),QLineEdit::Normal,0,&ok);

    if(ok&&!tag.isEmpty())
    {
        QString selectTag;
        QString select=QString("select distinct(rtag) from privateRecord where ruser='%1' and tag='%2'").arg(user).arg(tag);

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

void recordInsert::on_updateLabelBt_clicked()
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

void recordInsert::on_eraseLabelBt_clicked()
{
    QString deleteTag=ui->comboBox->currentText(); 
    int ok = QMessageBox::warning(this,tr("删除标签!"),tr("确认删除当前标签？"),QMessageBox::Yes,QMessageBox::No);

    if(ok == QMessageBox::Yes)
    {
        int index=ui->comboBox->currentIndex();
        ui->comboBox->removeItem(index);
    }
}

void recordInsert::on_okBt_clicked()
{
    // 获取用户输入
    QDate insertDate=ui->dateEdit->date();
    QString insertTag=ui->comboBox->currentText();
    QString insertText=ui->textEdit->toPlainText();

    if(insertText==NULL)
    {
        QMessageBox::warning(this,"warning","内容不能为空！",QMessageBox::Ok);
    }
    else { // 内容不为空
        // 获取插入的id
        int id=0;
        QString select=QString("select max(id) from privateRecord");

        query->exec(select);
        while (query->next()) {
            id=query->value(0).toInt();
        }
        id++;

        QString insert=QString("insert into privateRecord values('%1','%2','%3','%4','%5')").arg(insertTag).arg(insertText).arg(insertDate.toString("yyyy-MM-dd")).arg(user).arg(id);
        QString str=QString("select * from privateRecord where rdate='%1'and ruser='%2'").arg(insertDate.toString("yyyy-MM-dd")).arg(user);
        query->exec(insert);

        // 关闭插入窗口，记录tableview显示插入记录日期的所有记录
        privateWin->setRecordTV(str);
        privateWin->setSearchCB(); // 重置标签下拉框
        this->close();
    }
}

void recordInsert::on_cancelBt_clicked()
{
    this->close();
}
