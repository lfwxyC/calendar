#include "recordtext.h"
#include "ui_recordtext.h"

#include <QtSql/QSqlQuery>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>

recordText::recordText(QWidget *parent) : // 修改记录
    QDialog(parent),
    ui(new Ui::recordText)
{
    ui->setupUi(this);

    // 设置数据库
    db=QSqlDatabase::database("conn");
    query=new QSqlQuery(db);

    // 隐藏修改按钮
    ui->addLabelBt->hide();
    ui->updateLabelBt->hide();
    ui->eraseLabelBt->hide();
    ui->okBt->hide();
    ui->cancelBt->hide();
}

recordText::~recordText()
{
    delete ui;
}

void recordText::sendInfo(const QString &user, PrivateWindow *privateWin, int id) // 从上一个页面接收参数
{
    this->user=user;
    this->id=id;
    this->privateWin=privateWin;

    // 设置记录的数据
    QString select=QString("select * from privateRecord where id='%1'").arg(id);
    QString text;
    QDate date;
    QDate today=QDate::currentDate();

    query->exec(select);
    while (query->next()) {
        currentTag=query->value(0).toString();
        text=query->value(1).toString();
        date=query->value(2).toDate();
    }

    // 只读，不能修改
    ui->textEdit->setText(text);
    ui->textEdit->setReadOnly(true);
    ui->comboBox->addItem(currentTag);
    ui->comboBox->setEnabled(false);
    ui->dateEdit->setMaximumDate(today);
    ui->dateEdit->setDate(date);
    ui->dateEdit->setEnabled(false);
    ui->dateEdit->setCalendarPopup(true);
}

void recordText::on_editBt_clicked()
{
    // 显示修改按钮
    ui->addLabelBt->show();
    ui->updateLabelBt->show();
    ui->eraseLabelBt->show();
    ui->okBt->show();
    ui->cancelBt->show();
    ui->editBt->hide();

    // 可以修改
    ui->textEdit->setReadOnly(false);
    ui->comboBox->setEnabled(true);
    ui->dateEdit->setEnabled(true);

    // 设置标签下拉框
    QString select=QString("select distinct(rtag) from privateRecord where ruser='%1'").arg(user);

    query->exec(select);
    while (query->next()) {
        QString tag=query->value(0).toString();

        if(tag!=currentTag)
        {
            ui->comboBox->addItem(tag);
        }
    }
}

void recordText::on_addLabelBt_clicked()
{
    bool ok;
    QString tag=QInputDialog::getText(this,tr("新增标签"),tr("请输入标签名称"),QLineEdit::Normal,0,&ok);

    if(ok&&!tag.isEmpty())
    {
        ui->comboBox->addItem(tag);
        ui->comboBox->setCurrentText(tag);
    }
}

void recordText::on_updateLabelBt_clicked()
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

void recordText::on_eraseLabelBt_clicked()
{
    QString deleteTag=ui->comboBox->currentText();
    int ok = QMessageBox::warning(this,tr("删除标签!"),tr("确认删除当前标签？"),QMessageBox::Yes,QMessageBox::No);

    if(ok == QMessageBox::Yes)
    {
        int index=ui->comboBox->currentIndex();

        ui->comboBox->removeItem(index);
    }
}

void recordText::on_okBt_clicked()
{
    // 获取用户输入
    QDate updateDate=ui->dateEdit->date();
    QString updateTag=ui->comboBox->currentText();
    QString updateText=ui->textEdit->toPlainText();

    if(updateText==NULL)
    {
        QMessageBox::warning(this,"warning","内容不能为空！",QMessageBox::Ok);
    }
    else { // 内容不为空
        // 更新数据库中的数据
        QString update=QString("update privateRecord set rtag='%1',text='%2',rdate='%3'where id='%4'").arg(updateTag).arg(updateText).arg(updateDate.toString("yyyy-MM-dd")).arg(id);        
        query->exec(update);

        // 关闭窗口，重置记录
        QString str=QString("select * from privateRecord where rdate='%1'and ruser='%2'").arg(updateDate.toString("yyyy-MM-dd")).arg(user);
        privateWin->setRecordTV(str);
        privateWin->setSearchCB();
        this->close();
    }
}

void recordText::on_cancelBt_clicked()
{
    this->close();
}
