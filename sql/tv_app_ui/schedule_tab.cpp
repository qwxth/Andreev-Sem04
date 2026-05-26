#include "schedule_tab.h"
#include "db_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>

ScheduleTab::ScheduleTab(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Панель фильтров
    QHBoxLayout *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(new QLabel("Дата с:"));
    dtFrom = new QDateTimeEdit;
    dtFrom->setCalendarPopup(true);
    dtFrom->setDateTime(QDateTime::currentDateTime().addDays(-1));
    filterLayout->addWidget(dtFrom);

    filterLayout->addWidget(new QLabel("по:"));
    dtTo = new QDateTimeEdit;
    dtTo->setCalendarPopup(true);
    dtTo->setDateTime(QDateTime::currentDateTime().addDays(7));
    filterLayout->addWidget(dtTo);

    filterLayout->addWidget(new QLabel("Канал:"));
    cbChannel = new OneClickComboBox;
    cbChannel->setFocusPolicy(Qt::StrongFocus);
    cbChannel->clear();
    cbChannel->addItem("Все", 0);
    QSqlQuery q(DbManager::instance().getDatabase());
    if (q.exec("SELECT MIN(channel_id) as channel_id, full_name FROM tv.channels GROUP BY full_name ORDER BY full_name")) {
        while (q.next()) {
            cbChannel->addItem(q.value(1).toString(), q.value(0).toInt());
        }
    }
    filterLayout->addWidget(cbChannel);

    btnRefresh = new QPushButton("Обновить");
    filterLayout->addWidget(btnRefresh);
    mainLayout->addLayout(filterLayout);

    // Таблица
    table = new QTableWidget;
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"Дата/время", "Канал", "Передача", "Жанр", "Длит. (мин)", "Рейтинг"});
    table->horizontalHeader()->setStretchLastSection(true);
    mainLayout->addWidget(table);
    setLayout(mainLayout);

    connect(btnRefresh, &QPushButton::clicked, this, &ScheduleTab::refresh);
    connect(dtFrom, &QDateTimeEdit::dateTimeChanged, this, &ScheduleTab::refresh);
    connect(dtTo, &QDateTimeEdit::dateTimeChanged, this, &ScheduleTab::refresh);
    connect(cbChannel, qOverload<int>(&QComboBox::currentIndexChanged), this, &ScheduleTab::refresh);

    refresh();
}

ScheduleTab::~ScheduleTab() {}

void ScheduleTab::refresh() {
    QString from = dtFrom->dateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString to = dtTo->dateTime().toString("yyyy-MM-dd HH:mm:ss");
    int channelId = cbChannel->currentData().toInt();
    loadSchedule(from, to, channelId);
}

void ScheduleTab::loadSchedule(const QString& from, const QString& to, int channelId) {
    QString sql = "SELECT air_date_time, channel_name, title, genre, duration_minutes, rating "
                  "FROM tv.schedule_view "
                  "WHERE air_date_time BETWEEN '" + from + "' AND '" + to + "'";
    if (channelId > 0) sql += " AND channel_id = " + QString::number(channelId);
    sql += " ORDER BY air_date_time";

    QSqlQuery query(DbManager::instance().getDatabase());
    if (!query.exec(sql)) {
        QMessageBox::critical(this, "Ошибка", query.lastError().text());
        return;
    }

    table->setRowCount(0);
    int row = 0;
    while (query.next()) {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
        table->setItem(row, 4, new QTableWidgetItem(query.value(4).toString()));
        table->setItem(row, 5, new QTableWidgetItem(query.value(5).toString()));
        row++;
    }
    table->resizeColumnsToContents();
}
