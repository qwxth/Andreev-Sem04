#include "viewer_tab.h"
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

ViewerTab::ViewerTab(QWidget *parent) : QWidget(parent), currentViewerId(-1) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    table = new QTableWidget;
    mainLayout->addWidget(table);

    QHBoxLayout *searchLayout = new QHBoxLayout;
    leSearch = new QLineEdit;
    leSearch->setPlaceholderText("Поиск по фамилии");
    btnSearch = new QPushButton("Найти");
    searchLayout->addWidget(leSearch);
    searchLayout->addWidget(btnSearch);
    mainLayout->addLayout(searchLayout);

    QGroupBox *editGroup = new QGroupBox("Редактирование зрителя");
    QGridLayout *grid = new QGridLayout(editGroup);
    grid->addWidget(new QLabel("Фамилия:"), 0, 0);
    leLastName = new QLineEdit;
    grid->addWidget(leLastName, 0, 1);
    grid->addWidget(new QLabel("Имя:"), 0, 2);
    leFirstName = new QLineEdit;
    grid->addWidget(leFirstName, 0, 3);

    grid->addWidget(new QLabel("Дата рождения:"), 1, 0);
    deBirth = new QDateEdit;
    deBirth->setCalendarPopup(true);
    deBirth->setDate(QDate(1990,1,1));
    grid->addWidget(deBirth, 1, 1);
    grid->addWidget(new QLabel("Паспорт:"), 1, 2);
    lePassport = new QLineEdit;
    grid->addWidget(lePassport, 1, 3);

    grid->addWidget(new QLabel("Статус подписки:"), 2, 0);
    cbStatus = new QComboBox;
    cbStatus->addItems({"active", "vip", "blocked"});
    grid->addWidget(cbStatus, 2, 1);
    grid->addWidget(new QLabel("Телефон:"), 2, 2);
    lePhone = new QLineEdit;
    grid->addWidget(lePhone, 2, 3);

    grid->addWidget(new QLabel("Email:"), 3, 0);
    leEmail = new QLineEdit;
    grid->addWidget(leEmail, 3, 1);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnAdd = new QPushButton("Добавить");
    btnSave = new QPushButton("Сохранить");
    btnDelete = new QPushButton("Удалить");
    btnRefresh = new QPushButton("Обновить");
    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnDelete);
    btnLayout->addWidget(btnRefresh);
    grid->addLayout(btnLayout, 4, 0, 1, 4);

    mainLayout->addWidget(editGroup);
    setLayout(mainLayout);

    connect(btnRefresh, &QPushButton::clicked, this, &ViewerTab::refresh);
    connect(btnAdd, &QPushButton::clicked, this, &ViewerTab::onAdd);
    connect(btnSave, &QPushButton::clicked, this, &ViewerTab::onSave);
    connect(btnDelete, &QPushButton::clicked, this, &ViewerTab::onDelete);
    connect(btnSearch, &QPushButton::clicked, this, &ViewerTab::onSearch);
    connect(table, &QTableWidget::cellClicked, this, &ViewerTab::onRowSelected);

    refresh();
}

ViewerTab::~ViewerTab() {}

void ViewerTab::refresh() {
    loadViewers("");
    clearForm();
    setFormEnabled(false);
    currentViewerId = -1;
}

void ViewerTab::onSearch() {
    loadViewers(leSearch->text().trimmed());
}

void ViewerTab::loadViewers(const QString& lastNameFilter) {
    QString sql = "SELECT viewer_id, last_name, first_name, birth_date, passport_data, subscription_status, phone, email FROM tv.viewers";
    if (!lastNameFilter.isEmpty())
        sql += " WHERE last_name ILIKE '%" + lastNameFilter + "%'";
    sql += " ORDER BY last_name";
    QSqlQuery query(DbManager::instance().getDatabase());
    query.exec(sql);
    table->setRowCount(0);
    int row = 0;
    while (query.next()) {
        table->insertRow(row);
        for (int col = 0; col < 8; ++col)
            table->setItem(row, col, new QTableWidgetItem(query.value(col).toString()));
        row++;
    }
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({"ID","Фамилия","Имя","Дата рожд.","Паспорт","Статус","Телефон","Email"});
    table->hideColumn(0);
}

void ViewerTab::clearForm() {
    leLastName->clear();
    leFirstName->clear();
    deBirth->setDate(QDate(1990,1,1));
    lePassport->clear();
    cbStatus->setCurrentIndex(0);
    lePhone->clear();
    leEmail->clear();
}

void ViewerTab::setFormEnabled(bool en) {
    leLastName->setEnabled(en);
    leFirstName->setEnabled(en);
    deBirth->setEnabled(en);
    lePassport->setEnabled(en);
    cbStatus->setEnabled(en);
    lePhone->setEnabled(en);
    leEmail->setEnabled(en);
    btnSave->setEnabled(en);
    btnDelete->setEnabled(en);
}

void ViewerTab::onRowSelected(int row, int) {
    int id = table->item(row, 0)->text().toInt();
    currentViewerId = id;
    loadViewerToForm(id);
    setFormEnabled(true);
}

void ViewerTab::loadViewerToForm(int id) {
    QSqlQuery query(DbManager::instance().getDatabase());
    query.prepare("SELECT last_name, first_name, birth_date, passport_data, subscription_status, phone, email FROM tv.viewers WHERE viewer_id=?");
    query.bindValue(0, id);
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

void ViewerTab::onAdd() {
    clearForm();
    setFormEnabled(true);
    currentViewerId = -1;
    leLastName->setFocus();
}

void ViewerTab::onSave() {
    if (leLastName->text().trimmed().isEmpty() || leFirstName->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Фамилия и имя обязательны");
        return;
    }
    QSqlQuery query(DbManager::instance().getDatabase());
    if (currentViewerId == -1) {
        query.prepare("INSERT INTO tv.viewers (last_name, first_name, birth_date, passport_data, subscription_status, phone, email) VALUES (?,?,?,?,?,?,?)");
        query.bindValue(0, leLastName->text());
        query.bindValue(1, leFirstName->text());
        query.bindValue(2, deBirth->date());
        query.bindValue(3, lePassport->text());
        query.bindValue(4, cbStatus->currentText());
        query.bindValue(5, lePhone->text());
        query.bindValue(6, leEmail->text());
        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", query.lastError().text());
            return;
        }
        QMessageBox::information(this, "Успех", "Зритель добавлен");
    } else {
        query.prepare("UPDATE tv.viewers SET last_name=?, first_name=?, birth_date=?, passport_data=?, subscription_status=?, phone=?, email=? WHERE viewer_id=?");
        query.bindValue(0, leLastName->text());
        query.bindValue(1, leFirstName->text());
        query.bindValue(2, deBirth->date());
        query.bindValue(3, lePassport->text());
        query.bindValue(4, cbStatus->currentText());
        query.bindValue(5, lePhone->text());
        query.bindValue(6, leEmail->text());
        query.bindValue(7, currentViewerId);
        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", query.lastError().text());
            return;
        }
        QMessageBox::information(this, "Успех", "Данные зрителя обновлены");
    }
    refresh();
}

void ViewerTab::onDelete() {
    if (currentViewerId == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите зрителя");
        return;
    }
    if (QMessageBox::question(this, "Удаление", "Удалить зрителя?") == QMessageBox::Yes) {
        QSqlQuery query(DbManager::instance().getDatabase());
        query.prepare("DELETE FROM tv.viewers WHERE viewer_id=?");
        query.bindValue(0, currentViewerId);
        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", query.lastError().text());
            return;
        }
        refresh();
    }
}
