#include "viewer_personal_tab.h"
#include "db_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QTableWidget>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

ViewerPersonalTab::ViewerPersonalTab(int userId, QWidget *parent) : QWidget(parent), userId(userId), viewerId(-1) {
    // Получаем viewer_id из таблицы viewers по user_id
    QSqlQuery q(DbManager::instance().getDatabase());
    q.prepare("SELECT viewer_id FROM tv.viewers WHERE user_id = ?");
    q.bindValue(0, userId);
    if (q.exec() && q.next()) {
        viewerId = q.value(0).toInt();
    } else {
        // Если нет зрителя, создадим заглушку? Но лучше показать сообщение
        // Для простоты создадим временного зрителя с этим user_id
        q.prepare("INSERT INTO tv.viewers (last_name, first_name, user_id) VALUES ('Неизвестный', 'Пользователь', ?) RETURNING viewer_id");
        q.bindValue(0, userId);
        if (q.exec() && q.next()) {
            viewerId = q.value(0).toInt();
        }
    }

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Редактирование личной информации
    QGroupBox *groupInfo = new QGroupBox("Мои данные");
    QGridLayout *grid = new QGridLayout(groupInfo);
    grid->addWidget(new QLabel("Фамилия:"),0,0); leLastName = new QLineEdit; grid->addWidget(leLastName,0,1);
    grid->addWidget(new QLabel("Имя:"),0,2); leFirstName = new QLineEdit; grid->addWidget(leFirstName,0,3);
    grid->addWidget(new QLabel("Дата рождения:"),1,0); deBirth = new QDateEdit; deBirth->setCalendarPopup(true); grid->addWidget(deBirth,1,1);
    grid->addWidget(new QLabel("Паспорт:"),1,2); lePassport = new QLineEdit; grid->addWidget(lePassport,1,3);
    grid->addWidget(new QLabel("Статус подписки:"),2,0); cbStatus = new QComboBox; cbStatus->addItems({"active","vip","blocked"}); grid->addWidget(cbStatus,2,1);
    grid->addWidget(new QLabel("Телефон:"),2,2); lePhone = new QLineEdit; grid->addWidget(lePhone,2,3);
    grid->addWidget(new QLabel("Email:"),3,0); leEmail = new QLineEdit; grid->addWidget(leEmail,3,1);
    QPushButton *btnSave = new QPushButton("Сохранить");
    grid->addWidget(btnSave,4,0,1,2);
    mainLayout->addWidget(groupInfo);

    // Поиск просмотров по дате и статусу
    QGroupBox *groupSearch = new QGroupBox("Поиск просмотров");
    QHBoxLayout *searchLayout = new QHBoxLayout;
    leSearchDateFrom = new QLineEdit; leSearchDateFrom->setPlaceholderText("Дата от (ГГГГ-ММ-ДД)");
    leSearchDateTo = new QLineEdit; leSearchDateTo->setPlaceholderText("Дата до");
    leSearchStatus = new QLineEdit; leSearchStatus->setPlaceholderText("Статус (полностью/не досмотрено)");
    QPushButton *btnSearch = new QPushButton("Найти");
    searchLayout->addWidget(leSearchDateFrom);
    searchLayout->addWidget(leSearchDateTo);
    searchLayout->addWidget(leSearchStatus);
    searchLayout->addWidget(btnSearch);
    groupSearch->setLayout(searchLayout);
    mainLayout->addWidget(groupSearch);

    // Таблица просмотров
    table = new QTableWidget;
    mainLayout->addWidget(table);

    // Отчёты
    QHBoxLayout *reportLayout = new QHBoxLayout;
    QPushButton *btnReportAll = new QPushButton("Отчёт о всех просмотренных передачах");
    QPushButton *btnReportAccount = new QPushButton("Отчёт об операциях по счёту");
    reportLayout->addWidget(btnReportAll);
    reportLayout->addWidget(btnReportAccount);
    mainLayout->addLayout(reportLayout);

    setLayout(mainLayout);

    loadViewerData();

    connect(btnSave, &QPushButton::clicked, this, &ViewerPersonalTab::saveInfo);
    connect(btnSearch, &QPushButton::clicked, this, &ViewerPersonalTab::searchViews);
    connect(btnReportAll, &QPushButton::clicked, this, &ViewerPersonalTab::reportAllViews);
    connect(btnReportAccount, &QPushButton::clicked, this, &ViewerPersonalTab::reportAccountOperations);
}

ViewerPersonalTab::~ViewerPersonalTab() {}

void ViewerPersonalTab::loadViewerData() {
    if (viewerId == -1) {
        QMessageBox::warning(this, "Ошибка", "Не удалось найти профиль зрителя");
        return;
    }
    QSqlQuery query(DbManager::instance().getDatabase());
    query.prepare("SELECT last_name, first_name, birth_date, passport_data, subscription_status, phone, email FROM tv.viewers WHERE viewer_id=?");
    query.bindValue(0, viewerId);
    if (query.exec() && query.next()) {
        leLastName->setText(query.value(0).toString());
        leFirstName->setText(query.value(1).toString());
        deBirth->setDate(query.value(2).toDate());
        lePassport->setText(query.value(3).toString());
        cbStatus->setCurrentText(query.value(4).toString());
        lePhone->setText(query.value(5).toString());
        leEmail->setText(query.value(6).toString());
    }
}

void ViewerPersonalTab::saveInfo() {
    if (viewerId == -1) return;
    QSqlQuery query(DbManager::instance().getDatabase());
    query.prepare("UPDATE tv.viewers SET last_name=?, first_name=?, birth_date=?, passport_data=?, subscription_status=?, phone=?, email=? WHERE viewer_id=?");
    query.bindValue(0, leLastName->text());
    query.bindValue(1, leFirstName->text());
    query.bindValue(2, deBirth->date());
    query.bindValue(3, lePassport->text());
    query.bindValue(4, cbStatus->currentText());
    query.bindValue(5, lePhone->text());
    query.bindValue(6, leEmail->text());
    query.bindValue(7, viewerId);
    if (query.exec()) {
        QMessageBox::information(this, "Успех", "Данные обновлены");
    } else {
        QMessageBox::critical(this, "Ошибка", query.lastError().text());
    }
}

void ViewerPersonalTab::loadViews(const QString& dateFrom, const QString& dateTo, const QString& status) {
    if (viewerId == -1) return;
    QString sql = "SELECT b.title, vh.view_date, vh.watch_duration_minutes, b.duration_minutes FROM tv.view_history vh JOIN tv.broadcasts b ON vh.broadcast_id = b.broadcast_id WHERE vh.viewer_id = " + QString::number(viewerId);
    if (!dateFrom.isEmpty()) sql += " AND vh.view_date >= '" + dateFrom + "'";
    if (!dateTo.isEmpty()) sql += " AND vh.view_date <= '" + dateTo + "'";
    if (!status.isEmpty()) {
        if (status == "полностью") sql += " AND vh.watch_duration_minutes >= b.duration_minutes";
        else if (status == "не досмотрено") sql += " AND vh.watch_duration_minutes < b.duration_minutes";
    }
    sql += " ORDER BY vh.view_date DESC";
    QSqlQuery query(DbManager::instance().getDatabase());
    query.exec(sql);
    table->setRowCount(0);
    int row = 0;
    while (query.next()) {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        row++;
    }
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({"Передача","Дата просмотра","Длит. просмотра (мин)"});
}

void ViewerPersonalTab::searchViews() {
    QString dateFrom = leSearchDateFrom->text().trimmed();
    QString dateTo = leSearchDateTo->text().trimmed();
    QString status = leSearchStatus->text().trimmed();
    loadViews(dateFrom, dateTo, status);
}

void ViewerPersonalTab::reportAllViews() {
    if (viewerId == -1) return;
    QSqlQuery query(DbManager::instance().getDatabase());
    query.prepare("SELECT b.title, vh.view_date, vh.watch_duration_minutes FROM tv.view_history vh JOIN tv.broadcasts b ON vh.broadcast_id = b.broadcast_id WHERE vh.viewer_id = ? ORDER BY vh.view_date DESC");
    query.bindValue(0, viewerId);
    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка", query.lastError().text());
        return;
    }
    QString result;
    while (query.next()) {
        result += QString("%1 | %2 | %3 мин\n").arg(query.value(0).toString(), query.value(1).toString(), query.value(2).toString());
    }
    QMessageBox::information(this, "Все просмотренные передачи", result.isEmpty() ? "Нет данных" : result);
}

void ViewerPersonalTab::reportAccountOperations() {
    if (viewerId == -1) return;
    QSqlQuery query(DbManager::instance().getDatabase());
    query.prepare("SELECT change_date, old_status, new_status, comment FROM tv.subscription_history WHERE viewer_id = ? ORDER BY change_date DESC");
    query.bindValue(0, viewerId);
    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка", query.lastError().text());
        return;
    }
    QString result;
    while (query.next()) {
        result += QString("%1 | %2 → %3 | %4\n").arg(query.value(0).toString(), query.value(1).toString(), query.value(2).toString(), query.value(3).toString());
    }
    QMessageBox::information(this, "Операции по лицевому счёту", result.isEmpty() ? "Нет операций" : result);
}