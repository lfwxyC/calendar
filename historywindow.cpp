#include "historywindow.h"
#include "ui_historywindow.h"
#include "selectwindow.h"
#include "publicshow.h"
#include "evaluatewindow.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQueryModel>
#include <QDebug>
#include <QtCharts>
#include <QMessageBox>
#include <QDebug>

historyWindow::historyWindow(QWidget *parent) : // 历史待办页面
    QMainWindow(parent),
    ui(new Ui::historyWindow)
{
    ui->setupUi(this);

    // 设置数据库
    db=QSqlDatabase::database("conn");
    query=new QSqlQuery(db);

    // 只能选择今天之前的日期
    QDate today=QDate::currentDate();
    ui->todoCW->setMaximumDate(today);
    ui->joinCW->setMaximumDate(today);

    // 设置日历样式
    ui->todoCW->setWeekdayTextFormat(Qt::Saturday,ui->todoCW->weekdayTextFormat(Qt::Monday));
    ui->todoCW->setWeekdayTextFormat(Qt::Sunday,ui->todoCW->weekdayTextFormat(Qt::Monday));
    ui->todoCW->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);

    ui->joinCW->setWeekdayTextFormat(Qt::Saturday,ui->joinCW->weekdayTextFormat(Qt::Monday));
    ui->joinCW->setWeekdayTextFormat(Qt::Sunday,ui->joinCW->weekdayTextFormat(Qt::Monday));
    ui->joinCW->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);

    // 设置五星按钮透明
    ui->start1_1->setFlat(true);
    ui->start1_2->setFlat(true);
    ui->start1_3->setFlat(true);
    ui->start1_4->setFlat(true);
    ui->start1_5->setFlat(true);

    ui->start2_1->setFlat(true);
    ui->start2_2->setFlat(true);
    ui->start2_3->setFlat(true);
    ui->start2_4->setFlat(true);
    ui->start2_5->setFlat(true);

    // 隐藏评价按钮
    hideStar();
    ui->commitBt->hide();
}

historyWindow::~historyWindow()
{
    delete ui;
    delete query;
}

void historyWindow::hideStar() // 隐藏五颗星
{
    ui->start1_1->hide();
    ui->start1_2->hide();
    ui->start1_3->hide();
    ui->start1_4->hide();
    ui->start1_5->hide();

    ui->start2_1->hide();
    ui->start2_2->hide();
    ui->start2_3->hide();
    ui->start2_4->hide();
    ui->start2_5->hide();
}

void historyWindow::setTodoTV(const QString& str) // 设置私人待办
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

void historyWindow::setJoinTV(const QString &str) // 设置公开待办
{
    QSqlQueryModel *model=new QSqlQueryModel(ui->joinTV);

    model->setQuery(str,this->db);
    model->setHeaderData(0,Qt::Horizontal,tr("标签"));
    model->setHeaderData(1,Qt::Horizontal,tr("主题"));
    model->setHeaderData(2,Qt::Horizontal,tr("日期"));
    model->setHeaderData(3,Qt::Horizontal,tr("时间"));
    model->setHeaderData(4,Qt::Horizontal,tr("发起者"));
    model->setHeaderData(6,Qt::Horizontal,tr("具体描述"));
    ui->joinTV->setModel(model);
    ui->joinTV->setColumnHidden(5,true);

    // 获取第一行待办的数据
    QAbstractItemModel *modelTV = ui->joinTV->model ();
    QModelIndex idIndex = modelTV->index(0,5);
    QVariant idData = modelTV->data(idIndex);
    QString selectId=idData.toString();
    QString select=QString("select * from creator_user where tid='%1'").arg(selectId);

    // 显示第一行待办的参与成员
    model=new QSqlQueryModel(ui->joinMemberTV);
    model->setQuery(select,this->db);
    model->setHeaderData(1,Qt::Horizontal,tr("成员昵称"));
    model->setHeaderData(2,Qt::Horizontal,tr("成员邮箱"));
    ui->joinMemberTV->setModel(model);
    ui->joinMemberTV->setColumnHidden(0,true);
}

void historyWindow::on_todoCW_selectionChanged() // 点击日历，显示选中日期的待办
{
    QDate selectDate=ui->todoCW->selectedDate();
    QString str=QString("select * from privateTodo where date='%1'and user='%2'").arg(selectDate.toString("yyyy-MM-dd")).arg(user);
    setTodoTV(str);
}

void historyWindow::sendUser(const QString &user) // 从上一个页面接收数据
{
    this->user=user;

    // 右上角显示用户昵称
    QString select=QString("select name from user where email='%1'").arg(user);
    QString name;

    query->exec(select);
    while (query->next()) {
        name=query->value(0).toString();
    }

    ui->userBt->setText(name);
    ui->publicUserBt->setText(name);

    // 显示今天的待办
    QDate today=QDate::currentDate();
    QString str=QString("select * from privateTodo where date='%1'and user='%2'").arg(today.toString("yyyy-MM-dd")).arg(user);
    setTodoTV(str);

    str=QString("select publicTodo.* from publicTodo,creator_user where id=tid and date='%1'and memberEmail='%2'").arg(today.toString("yyyy-MM-dd")).arg(user);
    setJoinTV(str);

    // 设置标签下拉框
    select=QString("select distinct(ttag) from privateTodo where tuser='%1'").arg(user);

    query->exec(select);
    while (query->next()) {
        QString tag=query->value(0).toString();
        ui->todoSearchCB->addItem(tag);
    }

    select=QString("select distinct(tag) from publicTodo,creator_user where id=tid and memberEmail='%1'").arg(user);

    query->exec(select);
    while (query->next()) {
        QString tag=query->value(0).toString();
        ui->joinSearchCB->addItem(tag);
    }
}

void historyWindow::on_todoSelectBt_clicked() // 按标签查找私人待办
{
    QString selectTag=ui->todoSearchCB->currentText();
    QDate today=QDate::currentDate();
    QString str=QString("select * from privateTodo where ttag='%1'and tuser='%2'and tdate<'%3'").arg(selectTag).arg(user).arg(today.toString("yyyy-MM-dd"));
    setTodoTV(str);
}

void historyWindow::on_todoSelectAllBt_clicked() // 查看全部私人待办
{
    QDate today=QDate::currentDate();
    QString str=QString("select * from privateTodo where tuser='%1'and tdate<'%2'").arg(user).arg(today.toString("yyyy-MM-dd"));
    setTodoTV(str);
}

void historyWindow::showUser() // 返回个人主页，关闭当前页面
{
    SelectWindow *selectWin=new SelectWindow;
    selectWin->show();
    selectWin->sendUser(user);
    this->close();
}

void historyWindow::on_userBt_clicked()
{
    showUser();
}

void historyWindow::on_joinCW_selectionChanged() // 点击日历，显示选中日期的待办
{
    QDate selectDate=ui->joinCW->selectedDate();
    QString str=QString("select publicTodo.* from publicTodo,creator_user where id=tid and date='%1'and memberEmail='%2'").arg(selectDate.toString("yyyy-MM-dd")).arg(user);
    setJoinTV(str);
}

void historyWindow::on_joinSearchAllBt_clicked() // 显示全部公开待办
{
    QDate today=QDate::currentDate();
    QString str=QString("select publicTodo.* from publicTodo,creator_user where id=tid and memberEmail='%1'and date<'%2'").arg(user).arg(today.toString("yyyy-MM-dd"));
    setJoinTV(str);
}

void historyWindow::on_publicUserBt_clicked()
{
    showUser();
}

void historyWindow::on_start1_1_clicked() // 点击第一颗星，后面四颗星不亮
{
    hideStar();
    ui->start2_1->show();
    ui->start1_2->show();
    ui->start1_3->show();
    ui->start1_4->show();
    ui->start1_5->show();
}

void historyWindow::on_start1_2_clicked() // 点击第二颗星，后面三颗星不亮
{
    hideStar();
    ui->start2_1->show();
    ui->start2_2->show();
    ui->start1_3->show();
    ui->start1_4->show();
    ui->start1_5->show();
}

void historyWindow::on_start1_3_clicked() // 点击第三颗星，后面两颗星不亮
{
    hideStar();
    ui->start2_1->show();
    ui->start2_2->show();
    ui->start2_3->show();
    ui->start1_4->show();
    ui->start1_5->show();
}

void historyWindow::on_start1_4_clicked() // 点击第四颗星，最后一颗星不亮
{
    hideStar();
    ui->start2_1->show();
    ui->start2_2->show();
    ui->start2_3->show();
    ui->start2_4->show();
    ui->start1_5->show();
}

void historyWindow::on_start1_5_clicked() // 点击第五颗星，五颗星全亮
{
    hideStar();
    ui->start2_1->show();
    ui->start2_2->show();
    ui->start2_3->show();
    ui->start2_4->show();
    ui->start2_5->show();
}

void historyWindow::on_start2_1_clicked()
{
    on_start1_1_clicked();
}

void historyWindow::on_start2_2_clicked()
{
    on_start1_2_clicked();
}

void historyWindow::on_start2_3_clicked()
{
    on_start1_3_clicked();
}

void historyWindow::on_start2_4_clicked()
{
    on_start1_4_clicked();
}

void historyWindow::on_start2_5_clicked()
{
    on_start1_5_clicked();
}

void historyWindow::on_commitBt_clicked() // 提交评价
{ 
    // 获取选中待办的数据
    int currentRow = ui->joinTV->currentIndex().row();
    QAbstractItemModel *modelTV = ui->joinTV->model ();
    QModelIndex idIndex = modelTV->index(currentRow,5);
    QModelIndex dateIndex=modelTV->index(currentRow,2);
    QVariant idData = modelTV->data(idIndex);
    QVariant dateData=modelTV->data(dateIndex);
    int selectId=idData.toInt();
    QDate date=dateData.toDate();

    // 几颗星星亮了对应几分
    int score=0;
    if(!ui->start2_5->isHidden())
    {
        score=5;
    }
    else if (!ui->start2_4->isHidden()) {
        score=4;
    }
    else if (!ui->start2_3->isHidden()) {
        score=3;
    }
    else if (!ui->start2_2->isHidden()) {
        score=2;
    }
    else if (!ui->start2_1->isHidden()) {
        score=1;
    }

    // 把分数插入数据库中
    QString insert=QString("insert into historyEvaluate values('%1','%2','%3','%4')").arg(selectId).arg(user).arg(score).arg(date.toString("yyyy-MM-dd"));
    query->exec(insert);

    hideStar();
    ui->commitBt->hide();
}

void historyWindow::on_joinTV_clicked(const QModelIndex &index) // 点击待办显示对应参与成员
{
    ui->joinTV->setSelectionBehavior(QAbstractItemView::SelectRows); // 点击单元格选中整行

    // 获取对应待办的数据
    int currentRow = ui->joinTV->currentIndex().row();
    QAbstractItemModel *modelTV = ui->joinTV->model ();
    QModelIndex idIndex = modelTV->index(currentRow,5);
    QVariant idData = modelTV->data(idIndex);
    int selectId=idData.toInt();

    // 显示对应参与成员
    QString select=QString("select * from creator_user where tid='%1'").arg(selectId);

    QSqlQueryModel *model=new QSqlQueryModel(ui->joinMemberTV);
    model->setQuery(select,this->db);
    model->setHeaderData(1,Qt::Horizontal,tr("成员昵称"));
    model->setHeaderData(2,Qt::Horizontal,tr("成员邮箱"));
    ui->joinMemberTV->setModel(model);
    ui->joinMemberTV->setColumnHidden(0,true);
}

void historyWindow::on_evaluateBt_clicked() // 评价
{  
    // 获取选中待办的数据
    int currentRow = ui->joinTV->currentIndex().row();
    QAbstractItemModel *modelTV = ui->joinTV->model ();
    QModelIndex idIndex = modelTV->index(currentRow,5);
    QVariant idData = modelTV->data(idIndex);
    int selectId=idData.toInt();

    // 获取我的评分
    int myScore=0;
    QString selectMy=QString("select score from historyEvaluate where id='%1' and user='%2'").arg(selectId).arg(user);

    query->exec(selectMy);
    while (query->next()) {
        myScore=query->value(0).toInt();
    }

    if(myScore!=0)
    {
        QMessageBox::warning(this,"warning","不能重复评价！",QMessageBox::Ok);
        return;
    }

    // 显示评价按钮
    ui->start1_1->show();
    ui->start1_2->show();
    ui->start1_3->show();
    ui->start1_4->show();
    ui->start1_5->show();

    ui->commitBt->show();
}

void historyWindow::on_checkBt_clicked() // 查看评价
{
    // 获取选中待办的数据
    int currentRow = ui->joinTV->currentIndex().row();
    QAbstractItemModel *modelTV = ui->joinTV->model ();
    QModelIndex idIndex = modelTV->index(currentRow,5);
    QVariant idData = modelTV->data(idIndex);
    int selectId=idData.toInt();

    int score1=0,score2=0,score3=0,score4=0,score5=0,myScore=0; // score1-5:评分为1-5的人数， myScore:我的评分
    double sumScore=0; // 总评分
    QString selectMy=QString("select score from historyEvaluate where id='%1' and user='%2'").arg(selectId).arg(user);

    query->exec(selectMy);
    while (query->next()) {
        myScore=query->value(0).toInt();
    }

    // 从数据库中查找评分为1-5的人数以及总评分
    QString select1=QString("select count(*) from historyEvaluate where id='%1' and score=1").arg(selectId);
    QString select2=QString("select count(*) from historyEvaluate where id='%1' and score=2").arg(selectId);
    QString select3=QString("select count(*) from historyEvaluate where id='%1' and score=3").arg(selectId);
    QString select4=QString("select count(*) from historyEvaluate where id='%1' and score=4").arg(selectId);
    QString select5=QString("select count(*) from historyEvaluate where id='%1' and score=5").arg(selectId);
    QString selectSum=QString("select sum(score) from historyEvaluate where id='%1'").arg(selectId);

    query->exec(select1);
    while (query->next()) {
        score1=query->value(0).toInt();
    }

    query->exec(select2);
    while (query->next()) {
        score2=query->value(0).toInt();
    }

    query->exec(select3);
    while (query->next()) {
        score3=query->value(0).toInt();
    }

    query->exec(select4);
    while (query->next()) {
        score4=query->value(0).toUInt();
    }

    query->exec(select5);
    while (query->next()) {
        score5=query->value(0).toInt();
    }

    query->exec(selectSum);
    while (query->next()) {
        sumScore=query->value(0).toDouble();
    }

    double avgScore;
    if(score1==0&&score2==0&&score3==0&&score4==0&&score5==0) // 若没有人评分，则平均评分为0
    {
        avgScore=0;
    }
    else { // 没有参与评分的人不计入平均评分的计算
        avgScore=sumScore/(score1+score2+score3+score4+score5);
    }

    QString avgScoreStr=QString::asprintf("平均评分：%.2f",avgScore); // 输出平均评分，保留两位小数

    QBarSet *set0 = new QBarSet("他人评分");
    QBarSet *set1 = new QBarSet("我的评分");

    // 我的评分显示绿色，他人评分显示蓝色
    switch (myScore) {
    case 1:
        *set1<<score1<<0<<0<<0<<0;
        score1=0;
        break;
    case 2:
        *set1<<0<<score2<<0<<0<<0;
        score2=0;
        break;
    case 3:
        *set1<<0<<0<<score3<<0<<0;
        score3=0;
        break;
    case 4:
        *set1<<0<<0<<0<<score4<<0;
        score4=0;
        break;
    case 5:
        *set1<<0<<0<<0<<0<<score5;
        score5=0;
        break;
    }

    *set0 << score1 << score2 << score3 << score4 << score5; // 给他人评分赋值

    // 绘制柱状统计图
    QHorizontalBarSeries *series = new QHorizontalBarSeries();
    series->append(set0);
    series->append(set1);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(avgScoreStr);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QStringList categories;
    categories << "1分" << "2分" << "3分" << "4分" << "5分" ;
    QBarCategoryAxis *axisY = new QBarCategoryAxis();
    axisY->append(categories);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    QValueAxis *axisX = new QValueAxis();
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    axisX->applyNiceNumbers();

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // 在新窗口中显示柱状图
    evaluateWindow *evaluateWin=new evaluateWindow;
    evaluateWin->setCentralWidget(chartView);
    evaluateWin->resize(420, 300);
    evaluateWin->show();
}

void historyWindow::on_joinSearchBt_clicked() // 按标签查看公开待办
{
    QString selectTag=ui->joinSearchCB->currentText();
    QDate today=QDate::currentDate();
    QString str=QString("select publicTodo.* from publicTodo,creator_user where id=tid and tag='%1' and memberEmail='%2'and date<'%3'").arg(selectTag).arg(user).arg(today.toString("yyyy-MM-dd"));
    setJoinTV(str);
}
