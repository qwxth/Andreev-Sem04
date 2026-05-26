#include "broadcast_tab.h"
#include "db_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

BroadcastTab::BroadcastTab(QWidget *parent) : QWidget(parent), currentBroadcastId(-1) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Таблица
    table = new QTableWidget;
    mainLayout->addWidget(table);

    // Панель поиска (ID + название)
    QHBoxLayout *searchLayout = new QHBoxLayout;
    leSearchId = new QLineEdit;
    leSearchId->setPlaceholderText("ID передачи");
    btnSearchId = new QPushButton("Найти по ID");
    leSearchTitle = new QLineEdit;
    leSearchTitle->setPlaceholderText("Название передачи");
    btnSearchTitle = new QPushButton("Найти по названию");
    searchLayout->addWidget(new QLabel("Поиск:"));
    searchLayout->addWidget(leSearchId);
    searchLayout->addWidget(btnSearchId);
    searchLayout->addWidget(leSearchTitle);
    searchLayout->addWidget(btnSearchTitle);
    mainLayout->addLayout(searchLayout);

    // Форма редактирования
    QGroupBox *editGroup = new QGroupBox("Редактирование передачи");
    QGridLayout *grid = new QGridLayout(editGroup);

    grid->addWidget(new QLabel("Название:"), 0, 0);
    leTitle = new QLineEdit;
    grid->addWidget(leTitle, 0, 1);

    grid->addWidget(new QLabel("Жанр:"), 1, 0);
    cbGenre = new QComboBox;
    grid->addWidget(cbGenre, 1, 1);

    grid->addWidget(new QLabel("Длительность (мин):"), 2, 0);
    leDuration = new QLineEdit;
    grid->addWidget(leDuration, 2, 1);

    grid->addWidget(new QLabel("Дата и время эфира:"), 3, 0);
    dtAir = new QDateTimeEdit;
    dtAir->setCalendarPopup(true);
    grid->addWidget(dtAir, 3, 1);

    grid->addWidget(new QLabel("Рейтинг (0-10):"), 4, 0);
    leRating = new QLineEdit;
    grid->addWidget(leRating, 4, 1);

    grid->addWidget(new QLabel("Канал:"), 5, 0);
    cbChannel = new QComboBox;
    grid->addWidget(cbChannel, 5, 1);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnAdd = new QPushButton("Добавить");
    btnSave = new QPushButton("Сохранить");
    btnDelete = new QPushButton("Удалить");
    btnRefresh = new QPushButton("Обновить");
    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnDelete);
    btnLayout->addWidget(btnRefresh);
    grid->addLayout(btnLayout, 6, 0, 1, 2);

    mainLayout->addWidget(editGroup);
    setLayout(mainLayout);

    // Заполнение комбобоксов
    QSqlQuery q(DbManager::instance().getDatabase());
    q.exec("SELECT genre_id, genre_name FROM tv.genres ORDER BY genre_name");
    while (q.next()) cbGenre->addItem(q.value(1).toString(), q.value(0).toInt());
    q.exec("SELECT channel_id, full_name FROM tv.channels ORDER BY full_name");
    while (q.next()) cbChannel->addItem(q.value(1).toString(), q.value(0).toInt());

    // Подключения сигналов
    connect(btnRefresh, &QPushButton::clicked, this, &BroadcastTab::refresh);
    connect(btnAdd, &QPushButton::clicked, this, &BroadcastTab::onAdd);
    connect(btnSave, &QPushButton::clicked, this, &BroadcastTab::onSave);
    connect(btnDelete, &QPushButton::clicked, this, &BroadcastTab::onDelete);
    connect(btnSearchId, &QPushButton::clicked, this, &BroadcastTab::onSearchById);
    connect(btnSearchTitle, &QPushButton::clicked, this, &BroadcastTab::onSearchByTitle);
    connect(table, &QTableWidget::cellClicked, this, &BroadcastTab::onRowSelected);

    refresh();
}

BroadcastTab::~BroadcastTab() {}

void BroadcastTab::refresh() {
    loadBroadcasts();
    clearForm();
    setFormEnabled(false);
    currentBroadcastId = -1;
    leSearchId->clear();
    leSearchTitle->clear();
}

void BroadcastTab::onSearchById() {
    int id = leSearchId->text().toInt();
    loadBroadcasts(id, "");
}

void BroadcastTab::onSearchByTitle() {
    QString title = leSearchTitle->text().trimmed();
    loadBroadcasts(0, title);
}

void BroadcastTab::loadBroadcasts(int searchId, const QString& searchTitle) {
    QString sql = "SELECT b.broadcast_id, b.title, g.genre_name, b.duration_minutes, b.air_date_time, b.rating, c.full_name "
                  "FROM tv.broadcasts b "
                  "LEFT JOIN tv.genres g ON b.genre_id = g.genre_id "
                  "JOIN tv.channels c ON b.channel_id = c.channel_id";
    if (searchId > 0) {
        sql += " WHERE b.broadcast_id = " + QString::number(searchId);
    } else if (!searchTitle.isEmpty()) {
        sql += " WHERE b.title ILIKE '%" + searchTitle + "%'";
    }
    sql += " ORDER BY b.air_date_time";
    QSqlQuery query(DbManager::instance().getDatabase());
    query.exec(sql);
    table->setRowCount(0);
    int row = 0;
    while (query.next()) {
        table->insertRow(row);
        for (int col = 0; col < 7; ++col)
            table->setItem(row, col, new QTableWidgetItem(query.value(col).toString()));
        row++;
    }
    table->setColumnCount(7);
    table->setHorizontalHeaderLabels({"ID","Название","Жанр","Длит.","Дата эфира","Рейтинг","Канал"});
    table->hideColumn(0);
}

void BroadcastTab::clearForm() {
    leTitle->clear();
    leDuration->clear();
    leRating->clear();
    dtAir->setDateTime(QDateTime::currentDateTime());
    if (cbGenre->count()) cbGenre->setCurrentIndex(0);
    if (cbChannel->count()) cbChannel->setCurrentIndex(0);
}

void BroadcastTab::setFormEnabled(bool en) {
    leTitle->setEnabled(en);
    leDuration->setEnabled(en);
    leRating->setEnabled(en);
    dtAir->setEnabled(en);
    cbGenre->setEnabled(en);
    cbChannel->setEnabled(en);
    btnSave->setEnabled(en);
    btnDelete->setEnabled(en);
}

void BroadcastTab::onRowSelected(int row, int) {
    int id = table->item(row, 0)->text().toInt();
    currentBroadcastId = id;
    loadBroadcastToForm(id);
    setFormEnabled(true);
}

void BroadcastTab::loadBroadcastToForm(int id) {
    QSqlQuery query(DbManager::instance().getDatabase());
    query.prepare("SELECT title, genre_id, duration_minutes, air_date_time, rating, channel_id FROM tv.broadcasts WHERE broadcast_id=?");
    query.bindValue(0, id);
    if (query.exec() && query.next()) {
        leTitle->setText(query.value(0).toString());
        int genreId = query.value(1).toInt();
        int idx = cbGenre->findData(genreId);
        if (idx >= 0) cbGenre->setCurrentIndex(idx);
        leDuration->setText(query.value(2).toString());
        dtAir->setDateTime(query.value(3).toDateTime());
        leRating->setText(query.value(4).toString());
        int channelId = query.value(5).toInt();
        idx = cbChannel->findData(channelId);
        if (idx >= 0) cbChannel->setCurrentIndex(idx);
    }
}

void BroadcastTab::onAdd() {
    clearForm();
    setFormEnabled(true);
    currentBroadcastId = -1;
    leTitle->setFocus();
}

void BroadcastTab::onSave() {
    if (leTitle->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Название не может быть пустым");
        return;
    }
    QSqlQuery query(DbManager::instance().getDatabase());
    if (currentBroadcastId == -1) {
        query.prepare("INSERT INTO tv.broadcasts (title, genre_id, duration_minutes, air_date_time, rating, channel_id) "
                      "VALUES (?, ?, ?, ?, ?, ?)");
        query.bindValue(0, leTitle->text());
        query.bindValue(1, cbGenre->currentData());
        query.bindValue(2, leDuration->text().toInt());
        query.bindValue(3, dtAir->dateTime());
        query.bindValue(4, leRating->text().toDouble());
        query.bindValue(5, cbChannel->currentData());
        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", query.lastError().text());
            return;
        }
        QMessageBox::information(this, "Успех", "Передача добавлена");
    } else {
        query.prepare("UPDATE tv.broadcasts SET title=?, genre_id=?, duration_minutes=?, air_date_time=?, rating=?, channel_id=? WHERE broadcast_id=?");
        query.bindValue(0, leTitle->text());
        query.bindValue(1, cbGenre->currentData());
        query.bindValue(2, leDuration->text().toInt());
        query.bindValue(3, dtAir->dateTime());
        query.bindValue(4, leRating->text().toDouble());
        query.bindValue(5, cbChannel->currentData());
        query.bindValue(6, currentBroadcastId);
        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", query.lastError().text());
            return;
        }
        QMessageBox::information(this, "Успех", "Передача обновлена");
    }
    refresh();
}

void BroadcastTab::onDelete() {
    if (currentBroadcastId == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите передачу");
        return;
    }
    if (QMessageBox::question(this, "Удаление", "Удалить передачу?") == QMessageBox::Yes) {
        QSqlQuery query(DbManager::instance().getDatabase());
        query.prepare("DELETE FROM tv.broadcasts WHERE broadcast_id=?");
        query.bindValue(0, currentBroadcastId);
        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", query.lastError().text());
            return;
        }
        refresh();
    }
}
