#include "loginwindow.h"
#include "norwegianwoodstyle.h"

#include <QApplication>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QDate>
#include <QStyleFactory>
#include <QFile>

void connection() // 连接数据库
{
    QSqlDatabase db=QSqlDatabase::addDatabase("QSQLITE","conn");

    db.setDatabaseName("calendar.db"); // 设置数据库名称
    db.open();
}

int main(int argc, char *argv[])
{
    QApplication::setStyle(new NorwegianWoodStyle); // 设置风格

    QApplication a(argc, argv);

    connection();

    LoginWindow w;
    w.show(); // 显示登录页面
    return a.exec();
}
