#include "admin_tab.h"
#include "ui_admin_tab.h"
#include "db_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QDebug>

AdminTab::AdminTab(QWidget *parent) : QWidget(parent), ui(new Ui::AdminTab), currentChannelId(-1) {
    ui->setupUi(this);
    connect(ui->btnRefresh, &QPushButton::clicked, this, &AdminTab::refreshAll);
    connect(ui->btnAdd, &QPushButton::clicked, this, &AdminTab::onAdd);
    connect(ui->btnSave, &QPushButton::clicked, this, &AdminTab::onSave);
    connect(ui->btnDelete, &QPushButton::clicked, this, &AdminTab::onDelete);
    connect(ui->tableChannels, &QTableWidget::cellClicked, this, &AdminTab::onTableRowClicked);
    loadChannels();
    clearForm();
    setFormEnabled(false);
    ui->tableBroadcasts->setColumnCount(5);
    ui->tableBroadcasts->setHorizontalHeaderLabels({"ID", "Название", "Жанр", "Длит.(мин)", "Дата эфира"});
    ui->tableBroadcasts->hideColumn(0);
}

AdminTab::~AdminTab() {
    delete ui;
}

void AdminTab::refreshAll() {
    loadChannels();
    if (currentChannelId != -1)
        loadBroadcasts(currentChannelId);
}

void AdminTab::loadChannels() {
    if (!DbManager::instance().getDatabase().isOpen()) {
        QMessageBox::critical(this, "Ошибка", "Нет соединения с базой данных");
        return;
    }
    QSqlQuery query(DbManager::instance().getDatabase());
    if (!query.exec("SELECT channel_id, full_name, short_name, country, city, owner, contact_phone, contact_email FROM tv.channels ORDER BY channel_id")) {
        QMessageBox::critical(this, "Ошибка запроса", query.lastError().text());
        return;
    }
    ui->tableChannels->setRowCount(0);
    int row = 0;
    while (query.next()) {
        ui->tableChannels->insertRow(row);
        ui->tableChannels->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        ui->tableChannels->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        ui->tableChannels->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        ui->tableChannels->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
        ui->tableChannels->setItem(row, 4, new QTableWidgetItem(query.value(4).toString()));
        ui->tableChannels->setItem(row, 5, new QTableWidgetItem(query.value(5).toString()));
        ui->tableChannels->setItem(row, 6, new QTableWidgetItem(query.value(6).toString()));
        ui->tableChannels->setItem(row, 7, new QTableWidgetItem(query.value(7).toString()));
        row++;
    }
    ui->tableChannels->setColumnCount(8);
    ui->tableChannels->setHorizontalHeaderLabels({"ID", "Полное имя", "Кратко", "Страна", "Город", "Владелец", "Телефон", "Email"});
    ui->tableChannels->hideColumn(0);
}

void AdminTab::loadBroadcasts(int channelId) {
    if (!DbManager::instance().getDatabase().isOpen()) return;
    QSqlQuery query(DbManager::instance().getDatabase());
    query.prepare("SELECT broadcast_id, title, genre, duration_minutes, air_date_time FROM tv.broadcasts WHERE channel_id = ? ORDER BY air_date_time DESC");
    query.bindValue(0, channelId);
    if (!query.exec()) {
        qDebug() << "Ошибка загрузки передач:" << query.lastError().text();
        return;
    }
    ui->tableBroadcasts->setRowCount(0);
    int row = 0;
    while (query.next()) {
        ui->tableBroadcasts->insertRow(row);
        ui->tableBroadcasts->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        ui->tableBroadcasts->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        ui->tableBroadcasts->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        ui->tableBroadcasts->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
        ui->tableBroadcasts->setItem(row, 4, new QTableWidgetItem(query.value(4).toString()));
        row++;
    }
}

void AdminTab::onTableRowClicked(int row, int column) {
    Q_UNUSED(column);
    int id = ui->tableChannels->item(row, 0)->text().toInt();
    currentChannelId = id;
    loadChannelToForm(id);
    loadBroadcasts(id);
    setFormEnabled(true);
}

void AdminTab::clearForm() {
    ui->leFullName->clear();
    ui->leShortName->clear();
    ui->leCountry->clear();
    ui->leCity->clear();
    ui->leOwner->clear();
    ui->lePhone->clear();
    ui->leEmail->clear();
}

void AdminTab::setFormEnabled(bool enabled) {
    ui->leFullName->setEnabled(enabled);
    ui->leShortName->setEnabled(enabled);
    ui->leCountry->setEnabled(enabled);
    ui->leCity->setEnabled(enabled);
    ui->leOwner->setEnabled(enabled);
    ui->lePhone->setEnabled(enabled);
    ui->leEmail->setEnabled(enabled);
    ui->btnSave->setEnabled(enabled);
    ui->btnDelete->setEnabled(enabled);
}

void AdminTab::loadChannelToForm(int channelId) {
    if (!DbManager::instance().getDatabase().isOpen()) return;
    QSqlQuery query(DbManager::instance().getDatabase());
    query.prepare("SELECT full_name, short_name, country, city, owner, contact_phone, contact_email FROM tv.channels WHERE channel_id = ?");
    query.bindValue(0, channelId);
    if (query.exec() && query.next()) {
        ui->leFullName->setText(query.value(0).toString());
        ui->leShortName->setText(query.value(1).toString());
        ui->leCountry->setText(query.value(2).toString());
        ui->leCity->setText(query.value(3).toString());
        ui->leOwner->setText(query.value(4).toString());
        ui->lePhone->setText(query.value(5).toString());
        ui->leEmail->setText(query.value(6).toString());
    }
}

void AdminTab::onAdd() {
    clearForm();
    setFormEnabled(true);
    currentChannelId = -1;
    ui->leFullName->setFocus();
    QMessageBox::information(this, "Добавление", "Введите данные нового канала и нажмите «Сохранить»");
}

void AdminTab::onSave() {
    if (!DbManager::instance().getDatabase().isOpen()) {
        QMessageBox::critical(this, "Ошибка", "Нет соединения с БД");
        return;
    }
    // Проверим заполнение обязательных полей (например, полное имя)
    if (ui->leFullName->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Поле «Полное название» не может быть пустым");
        return;
    }
    QSqlQuery query(DbManager::instance().getDatabase());
    if (currentChannelId == -1) {
        // Вставка нового канала
        query.prepare("INSERT INTO tv.channels (full_name, short_name, country, city, owner, contact_phone, contact_email) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?)");
        query.bindValue(0, ui->leFullName->text());
        query.bindValue(1, ui->leShortName->text());
        query.bindValue(2, ui->leCountry->text());
        query.bindValue(3, ui->leCity->text());
        query.bindValue(4, ui->leOwner->text());
        query.bindValue(5, ui->lePhone->text());
        query.bindValue(6, ui->leEmail->text());
        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", query.lastError().text());
            return;
        }
        QMessageBox::information(this, "Успех", "Канал добавлен");
        loadChannels();
        clearForm();
        setFormEnabled(false);
        currentChannelId = -1;
        ui->tableBroadcasts->setRowCount(0); // очистить таблицу передач
    } else {
        // Обновление существующего
        query.prepare("UPDATE tv.channels SET full_name=?, short_name=?, country=?, city=?, owner=?, contact_phone=?, contact_email=? WHERE channel_id=?");
        query.bindValue(0, ui->leFullName->text());
        query.bindValue(1, ui->leShortName->text());
        query.bindValue(2, ui->leCountry->text());
        query.bindValue(3, ui->leCity->text());
        query.bindValue(4, ui->leOwner->text());
        query.bindValue(5, ui->lePhone->text());
        query.bindValue(6, ui->leEmail->text());
        query.bindValue(7, currentChannelId);
        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", query.lastError().text());
            return;
        }
        QMessageBox::information(this, "Успех", "Данные канала обновлены");
        loadChannels();
        loadChannelToForm(currentChannelId);
        loadBroadcasts(currentChannelId);
    }
}

void AdminTab::onDelete() {
    if (currentChannelId == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите канал для удаления");
        return;
    }
    if (QMessageBox::question(this, "Удаление", "Удалить канал и все его передачи? Это действие необратимо.") == QMessageBox::Yes) {
        QSqlQuery query(DbManager::instance().getDatabase());
        query.prepare("DELETE FROM tv.channels WHERE channel_id=?");
        query.bindValue(0, currentChannelId);
        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", query.lastError().text());
            return;
        }
        QMessageBox::information(this, "Успех", "Канал удалён");
        loadChannels();
        clearForm();
        setFormEnabled(false);
        currentChannelId = -1;
        ui->tableBroadcasts->setRowCount(0);
    }
}
