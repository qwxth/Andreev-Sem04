#include "channel_employee_tab.h"
#include "db_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QTableWidget>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

ChannelEmployeeTab::ChannelEmployeeTab(int userId, QWidget *parent) : QWidget(parent), userId(userId), channelId(-1) {
    // Получаем channel_id из таблицы channel_employees
    QSqlQuery q(DbManager::instance().getDatabase());
    q.prepare("SELECT channel_id FROM tv.channel_employees WHERE user_id = ?");
    q.bindValue(0, userId);
    if (q.exec() && q.next()) channelId = q.value(0).toInt();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Редактирование своего канала
    QGroupBox *groupChannel = new QGroupBox("Мой телеканал");
    QGridLayout *grid = new QGridLayout(groupChannel);
    grid->addWidget(new QLabel("Полное название:"),0,0); leFullName = new QLineEdit; grid->addWidget(leFullName,0,1);
    grid->addWidget(new QLabel("Краткое:"),1,0); leShortName = new QLineEdit; grid->addWidget(leShortName,1,1);
    grid->addWidget(new QLabel("Страна:"),2,0); leCountry = new QLineEdit; grid->addWidget(leCountry,2,1);
    grid->addWidget(new QLabel("Город:"),3,0); leCity = new QLineEdit; grid->addWidget(leCity,3,1);
    grid->addWidget(new QLabel("Владелец:"),4,0); leOwner = new QLineEdit; grid->addWidget(leOwner,4,1);
    grid->addWidget(new QLabel("Телефон:"),5,0); lePhone = new QLineEdit; grid->addWidget(lePhone,5,1);
    grid->addWidget(new QLabel("Email:"),6,0); leEmail = new QLineEdit; grid->addWidget(leEmail,6,1);
    QPushButton *btnSaveChannel = new QPushButton("Сохранить изменения");
    grid->addWidget(btnSaveChannel,7,0,1,2);
    mainLayout->addWidget(groupChannel);

    // Поиск передач
    QGroupBox *groupSearch = new QGroupBox("Поиск передач");
    QHBoxLayout *searchLayout = new QHBoxLayout;
    leBroadcastId = new QLineEdit; leBroadcastId->setPlaceholderText("ID");
    leDateFrom = new QDateTimeEdit; leDateFrom->setCalendarPopup(true);
    leDateTo = new QDateTimeEdit; leDateTo->setCalendarPopup(true);
    leRatingFrom = new QLineEdit; leRatingFrom->setPlaceholderText("Рейтинг от");
    leRatingTo = new QLineEdit; leRatingTo->setPlaceholderText("до");
    QPushButton *btnSearch = new QPushButton("Найти");
    searchLayout->addWidget(new QLabel("ID:")); searchLayout->addWidget(leBroadcastId);
    searchLayout->addWidget(new QLabel("Дата с:")); searchLayout->addWidget(leDateFrom);
    searchLayout->addWidget(new QLabel("по:")); searchLayout->addWidget(leDateTo);
    searchLayout->addWidget(new QLabel("Рейтинг:")); searchLayout->addWidget(leRatingFrom);
    searchLayout->addWidget(leRatingTo);
    searchLayout->addWidget(btnSearch);
    groupSearch->setLayout(searchLayout);
    mainLayout->addWidget(groupSearch);

    // Таблица результатов
    table = new QTableWidget;
    mainLayout->addWidget(table);

    // Отчёты
    QHBoxLayout *reportLayout = new QHBoxLayout;
    QPushButton *btnReport1 = new QPushButton("Отчёт о передачах канала со зрителями");
    QPushButton *btnReport2 = new QPushButton("Отчёт о зрителях канала");
    reportLayout->addWidget(btnReport1);
    reportLayout->addWidget(btnReport2);
    mainLayout->addLayout(reportLayout);

    setLayout(mainLayout);

    loadChannelData();

    connect(btnSaveChannel, &QPushButton::clicked, this, &ChannelEmployeeTab::saveChannel);
    connect(btnSearch, &QPushButton::clicked, this, &ChannelEmployeeTab::searchBroadcasts);
    connect(btnReport1, &QPushButton::clicked, this, &ChannelEmployeeTab::reportBroadcastsWithViewers);
    connect(btnReport2, &QPushButton::clicked, this, &ChannelEmployeeTab::reportViewersOfChannel);
}

void ChannelEmployeeTab::loadChannelData() {
    if (channelId == -1) return;
    QSqlQuery query(DbManager::instance().getDatabase());
    query.prepare("SELECT full_name, short_name, country, city, owner, contact_phone, contact_email FROM tv.channels WHERE channel_id=?");
    query.bindValue(0, channelId);
    if (query.exec() && query.next()) {
        leFullName->setText(query.value(0).toString());
        leShortName->setText(query.value(1).toString());
        leCountry->setText(query.value(2).toString());
        leCity->setText(query.value(3).toString());
        leOwner->setText(query.value(4).toString());
        lePhone->setText(query.value(5).toString());
        leEmail->setText(query.value(6).toString());
    }
}

void ChannelEmployeeTab::saveChannel() {
    if (channelId == -1) {
        QMessageBox::warning(this, "Ошибка", "Канал не найден");
        return;
    }
    QSqlQuery query(DbManager::instance().getDatabase());
    query.prepare("UPDATE tv.channels SET full_name=?, short_name=?, country=?, city=?, owner=?, contact_phone=?, contact_email=? WHERE channel_id=?");
    query.bindValue(0, leFullName->text());
    query.bindValue(1, leShortName->text());
    query.bindValue(2, leCountry->text());
    query.bindValue(3, leCity->text());
    query.bindValue(4, leOwner->text());
    query.bindValue(5, lePhone->text());
    query.bindValue(6, leEmail->text());
    query.bindValue(7, channelId);
    if (query.exec()) QMessageBox::information(this, "Успех", "Данные канала обновлены");
    else QMessageBox::critical(this, "Ошибка", query.lastError().text());
}

void ChannelEmployeeTab::searchBroadcasts() {
    int bid = leBroadcastId->text().toInt();
    QDateTime from = leDateFrom->dateTime();
    QDateTime to = leDateTo->dateTime();
    double ratingFrom = leRatingFrom->text().toDouble();
    double ratingTo = leRatingTo->text().toDouble();
    loadBroadcasts(bid > 0 ? QString::number(bid) : "", from, to, ratingFrom, ratingTo);
}

void ChannelEmployeeTab::loadBroadcasts(const QString& broadcastId, const QDateTime& from, const QDateTime& to, double ratingFrom, double ratingTo) {
    QString sql = "SELECT b.broadcast_id, b.title, b.air_date_time, b.rating FROM tv.broadcasts b WHERE b.channel_id = " + QString::number(channelId);
    if (!broadcastId.isEmpty()) sql += " AND b.broadcast_id = " + broadcastId;
    if (from.isValid()) sql += " AND b.air_date_time >= '" + from.toString("yyyy-MM-dd HH:mm:ss") + "'";
    if (to.isValid()) sql += " AND b.air_date_time <= '" + to.toString("yyyy-MM-dd HH:mm:ss") + "'";
    if (ratingFrom > 0) sql += " AND b.rating >= " + QString::number(ratingFrom);
    if (ratingTo > 0) sql += " AND b.rating <= " + QString::number(ratingTo);
    sql += " ORDER BY b.air_date_time";

    QSqlQuery query(DbManager::instance().getDatabase());
    query.exec(sql);
    table->setRowCount(0);
    int row = 0;
    while (query.next()) {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
        row++;
    }
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"ID","Название","Дата эфира","Рейтинг"});
}

void ChannelEmployeeTab::reportBroadcastsWithViewers() {
    QSqlQuery query(DbManager::instance().getDatabase());
    query.prepare("SELECT b.title, COUNT(DISTINCT vh.viewer_id) as viewers_count FROM tv.broadcasts b LEFT JOIN tv.view_history vh ON b.broadcast_id = vh.broadcast_id WHERE b.channel_id = ? GROUP BY b.broadcast_id ORDER BY b.air_date_time");
    query.bindValue(0, channelId);
    QString result;
    while (query.next()) {
        result += QString("%1 | Зрителей: %2\n").arg(query.value(0).toString(), query.value(1).toString());
    }
    QMessageBox::information(this, "Отчёт", result.isEmpty() ? "Нет данных" : result);
}

void ChannelEmployeeTab::reportViewersOfChannel() {
    QSqlQuery query(DbManager::instance().getDatabase());
    query.prepare("SELECT DISTINCT v.last_name, v.first_name, v.phone FROM tv.view_history vh JOIN tv.viewers v ON vh.viewer_id = v.viewer_id JOIN tv.broadcasts b ON vh.broadcast_id = b.broadcast_id WHERE b.channel_id = ? ORDER BY v.last_name");
    query.bindValue(0, channelId);
    QString result;
    while (query.next()) {
        result += QString("%1 %2 | Тел: %3\n").arg(query.value(0).toString(), query.value(1).toString(), query.value(2).toString());
    }
    QMessageBox::information(this, "Отчёт", result.isEmpty() ? "Нет зрителей" : result);
}
