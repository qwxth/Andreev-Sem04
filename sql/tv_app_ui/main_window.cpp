#include "main_window.h"
#include "db_manager.h"
#include "admin_tab.h"
#include "broadcast_tab.h"
#include "viewer_tab.h"
#include "view_history_tab.h"
#include "reports_tab.h"
#include "schedule_tab.h"
#include "channel_employee_tab.h"
#include "viewer_personal_tab.h"
#include "supervisor_tab.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QSqlQuery>
#include <QMessageBox>

MainWindow::MainWindow(const QString& role, int userId, const QString& username, QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QString("АИС Учёт телепередач – %1 (%2)").arg(username, role));
    setGeometry(100, 100, 1300, 800);
    QTabWidget *tabs = new QTabWidget(this);
    setCentralWidget(tabs);

    if (role == "admin") {
        tabs->addTab(new AdminTab(this), "Телеканалы");
        tabs->addTab(new BroadcastTab(this), "Телепередачи");
        tabs->addTab(new ViewerTab(this), "Пользователи");
        tabs->addTab(new ViewHistoryTab(this), "Просмотры");
        tabs->addTab(new ReportsTab(0, this), "Отчёты");
        tabs->addTab(new ScheduleTab(this), "График передач");
    }
    else if (role == "channel_employee") {
        tabs->addTab(new ChannelEmployeeTab(userId, this), "Мой канал");
        tabs->addTab(new ScheduleTab(this), "График передач");
        tabs->addTab(new ReportsTab(0, this), "Отчёты");
    }
    else if (role == "viewer") {
        tabs->addTab(new ViewerPersonalTab(userId, this), "Мои просмотры");
        tabs->addTab(new ScheduleTab(this), "График передач");
        tabs->addTab(new ReportsTab(userId, this), "Отчёты");
    }
    else if (role == "supervisor") {
        tabs->addTab(new SupervisorTab(this), "Надзор");
        tabs->addTab(new ScheduleTab(this), "График передач");
        tabs->addTab(new ReportsTab(0, this), "Отчёты");
    }
}

MainWindow::~MainWindow() {}