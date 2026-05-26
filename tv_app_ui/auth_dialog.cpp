#include "auth_dialog.h"
#include "ui_auth_dialog.h"
#include "db_manager.h"
#include "utils.h"
#include <QMessageBox>
#include <QSqlQuery>

AuthDialog::AuthDialog(QWidget *parent) : QDialog(parent), ui(new Ui::AuthDialog), userId(-1) {
    ui->setupUi(this);
    connect(ui->btnLogin, &QPushButton::clicked, this, &AuthDialog::onLogin);
}

AuthDialog::~AuthDialog() {
    delete ui;
}

int AuthDialog::getUserId() const { return userId; }
QString AuthDialog::getUserRole() const { return userRole; }
QString AuthDialog::getUserName() const { return username; }

void AuthDialog::onLogin() {
    QString u = ui->leUsername->text().trimmed();
    QString p = ui->lePassword->text().trimmed();
    if (u.isEmpty() || p.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните оба поля");
        return;
    }

    QString hashed = hashPassword(p);
    QSqlQuery query(DbManager::instance().getDatabase());
    query.prepare("SELECT u.user_id, r.role_name FROM tv.users u JOIN tv.roles r ON u.role_id = r.role_id WHERE u.username = :user AND u.password_hash = :hash");
    query.bindValue(":user", u);
    query.bindValue(":hash", hashed);
    if (query.exec() && query.next()) {
        userId = query.value(0).toInt();
        userRole = query.value(1).toString();
        username = u;
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка", "Неверный логин или пароль");
    }
}