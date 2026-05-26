#include "broadcasts_dialog.h"
#include "ui_broadcasts_dialog.h"
#include <QSqlError>
#include "db_manager.h"
#include <QSqlQuery>
#include <QTableWidgetItem>
#include <QMessageBox>

BroadcastsDialog::BroadcastsDialog(int channelId, QWidget *parent) :
    QDialog(parent), ui(new Ui::BroadcastsDialog), channelId(channelId) {
    ui->setupUi(this);
    connect(ui->btnClose, &QPushButton::clicked, this, &QDialog::close);
    loadBroadcasts();
}

BroadcastsDialog::~BroadcastsDialog() {
    delete ui;
}

void BroadcastsDialog::loadBroadcasts() {
    if (!DbManager::instance().getDatabase().isOpen()) {
        QMessageBox::critical(this, "Ошибка", "Нет соединения с БД");
        return;
    }
    QSqlQuery query(DbManager::instance().getDatabase());
    query.prepare("SELECT broadcast_id, title, genre, duration_minutes, air_date_time, rating FROM tv.broadcasts WHERE channel_id = ?");
    query.bindValue(0, channelId);
    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка", query.lastError().text());
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
        ui->tableBroadcasts->setItem(row, 5, new QTableWidgetItem(query.value(5).toString()));
        row++;
    }
    ui->tableBroadcasts->setColumnCount(6);
    ui->tableBroadcasts->setHorizontalHeaderLabels({"ID", "Название", "Жанр", "Длит.(мин)", "Дата эфира", "Рейтинг"});
    ui->tableBroadcasts->hideColumn(0);
}
