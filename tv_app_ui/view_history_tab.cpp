#include "view_history_tab.h"
#include "db_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

ViewHistoryTab::ViewHistoryTab(QWidget *parent) : QWidget(parent), currentViewId(-1) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Панель поиска по зрителю
    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->addWidget(new QLabel("ID зрителя:"));
    leViewerId = new QLineEdit;
    btnSearch = new QPushButton("Найти");
    searchLayout->addWidget(leViewerId);
    searchLayout->addWidget(btnSearch);
    mainLayout->addLayout(searchLayout);

    // Таблица
    table = new QTableWidget;
    mainLayout->addWidget(table);

    // Кнопки
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnAdd = new QPushButton("Добавить просмотр");
    btnDelete = new QPushButton("Удалить");
    btnRefresh = new QPushButton("Обновить");
    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnDelete);
    btnLayout->addWidget(btnRefresh);
    mainLayout->addLayout(btnLayout);

    setLayout(mainLayout);

    connect(btnRefresh, &QPushButton::clicked, this, &ViewHistoryTab::refresh);
    connect(btnAdd, &QPushButton::clicked, this, &ViewHistoryTab::onAdd);
    connect(btnDelete, &QPushButton::clicked, this, &ViewHistoryTab::onDelete);
    connect(btnSearch, &QPushButton::clicked, this, &ViewHistoryTab::onSearch);
    connect(table, &QTableWidget::cellClicked, this, &ViewHistoryTab::onRowSelected);

    refresh();
}

ViewHistoryTab::~ViewHistoryTab() {}

void ViewHistoryTab::refresh() {
    loadHistory();
    leViewerId->clear();
}

void ViewHistoryTab::onSearch() {
    int viewerId = leViewerId->text().toInt();
    loadHistory(viewerId);
}

void ViewHistoryTab::loadHistory(int viewerId) {
    QString sql = "SELECT vh.view_id, v.last_name || ' ' || v.first_name, b.title, vh.view_date, vh.watch_duration_minutes, vh.device "
                  "FROM tv.view_history vh "
                  "JOIN tv.viewers v ON vh.viewer_id = v.viewer_id "
                  "JOIN tv.broadcasts b ON vh.broadcast_id = b.broadcast_id";
    if (viewerId > 0) {
        sql += " WHERE vh.viewer_id = " + QString::number(viewerId);
    }
    sql += " ORDER BY vh.view_date DESC";
    QSqlQuery query(DbManager::instance().getDatabase());
    query.exec(sql);
    table->setRowCount(0);
    int row = 0;
    while (query.next()) {
        table->insertRow(row);
        for (int col = 0; col < 6; ++col)
            table->setItem(row, col, new QTableWidgetItem(query.value(col).toString()));
        row++;
    }
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"ID","Зритель","Передача","Дата просмотра","Длит. (мин)","Устройство"});
    table->hideColumn(0);
}

void ViewHistoryTab::clearForm() {
    currentViewId = -1;
}

void ViewHistoryTab::onRowSelected(int row, int) {
    currentViewId = table->item(row, 0)->text().toInt();
}

void ViewHistoryTab::onAdd() {
    // Упрощённо – можно создать диалог, но для демонстрации оставим сообщение
    QMessageBox::information(this, "Добавление", "Функция добавления просмотра будет реализована в диалоге");
}

void ViewHistoryTab::onDelete() {
    if (currentViewId == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите запись");
        return;
    }
    if (QMessageBox::question(this, "Удаление", "Удалить просмотр?") == QMessageBox::Yes) {
        QSqlQuery query(DbManager::instance().getDatabase());
        query.prepare("DELETE FROM tv.view_history WHERE view_id=?");
        query.bindValue(0, currentViewId);
        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", query.lastError().text());
            return;
        }
        refresh();
    }
}
