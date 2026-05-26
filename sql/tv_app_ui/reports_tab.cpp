#include "reports_tab.h"
#include "db_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QInputDialog>
#include <QMessageBox>
#include <QDialog>
#include <QComboBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMouseEvent>
#include <QSqlQuery>
#include <QSqlError>

class OneClickComboBox : public QComboBox {
public:
    using QComboBox::QComboBox;
    void mousePressEvent(QMouseEvent *e) override {
        if (e->button() == Qt::LeftButton) showPopup();
        else QComboBox::mousePressEvent(e);
    }
};

static QString getItemOneClick(const QString& title, const QString& label, const QStringList& items, int current = 0, bool *ok = nullptr) {
    QDialog dlg;
    dlg.setWindowTitle(title);
    QVBoxLayout layout(&dlg);
    layout.addWidget(new QLabel(label));
    OneClickComboBox *combo = new OneClickComboBox;
    combo->addItems(items);
    if (current >= 0 && current < items.size()) combo->setCurrentIndex(current);
    layout.addWidget(combo);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout.addWidget(buttonBox);
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() == QDialog::Accepted) {
        if (ok) *ok = true;
        return combo->currentText();
    }
    if (ok) *ok = false;
    return QString();
}

ReportsTab::ReportsTab(int currentUserId, QWidget *parent) : QWidget(parent), currentUserId(currentUserId) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *btnLayout = new QHBoxLayout;

    QPushButton *btnChannelsByCity = new QPushButton("Каналы в городе");
    QPushButton *btnChannelWithBroadcasts = new QPushButton("Канал с передачами");
    QPushButton *btnViewersByStatus = new QPushButton("Зрители по статусу");
    QPushButton *btnViewerHistory = new QPushButton("История зрителя");
    QPushButton *btnBroadcastViewers = new QPushButton("Зрители передачи");
    QPushButton *btnChannelBroadcastsViewers = new QPushButton("Передачи канала со зрителями");
    QPushButton *btnChannelViewers = new QPushButton("Зрители канала");
    QPushButton *btnMyBroadcasts = new QPushButton("Мои просмотры");
    QPushButton *btnMyAccount = new QPushButton("Операции по счёту");

    btnLayout->addWidget(btnChannelsByCity);
    btnLayout->addWidget(btnChannelWithBroadcasts);
    btnLayout->addWidget(btnViewersByStatus);
    btnLayout->addWidget(btnViewerHistory);
    btnLayout->addWidget(btnBroadcastViewers);
    btnLayout->addWidget(btnChannelBroadcastsViewers);
    btnLayout->addWidget(btnChannelViewers);
    btnLayout->addWidget(btnMyBroadcasts);
    btnLayout->addWidget(btnMyAccount);

    mainLayout->addLayout(btnLayout);
    textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    mainLayout->addWidget(textEdit);
    setLayout(mainLayout);

    connect(btnChannelsByCity, &QPushButton::clicked, this, &ReportsTab::reportChannelsByCity);
    connect(btnChannelWithBroadcasts, &QPushButton::clicked, this, &ReportsTab::reportChannelWithBroadcasts);
    connect(btnViewersByStatus, &QPushButton::clicked, this, &ReportsTab::reportViewersByStatus);
    connect(btnViewerHistory, &QPushButton::clicked, this, &ReportsTab::reportViewerHistory);
    connect(btnBroadcastViewers, &QPushButton::clicked, this, &ReportsTab::reportBroadcastViewers);
    connect(btnChannelBroadcastsViewers, &QPushButton::clicked, this, &ReportsTab::reportChannelBroadcastsViewers);
    connect(btnChannelViewers, &QPushButton::clicked, this, &ReportsTab::reportChannelViewers);
    connect(btnMyBroadcasts, &QPushButton::clicked, this, &ReportsTab::reportMyBroadcasts);
    connect(btnMyAccount, &QPushButton::clicked, this, &ReportsTab::reportMyAccount);
}

void ReportsTab::showReport(const QString& title, const QString& content) {
    textEdit->setHtml("<h2>" + title + "</h2><pre>" + content + "</pre>");
}

void ReportsTab::reportChannelsByCity() {
    QString city = QInputDialog::getText(this, "Город", "Введите город:");
    if (city.isEmpty()) return;
    QSqlQuery query(DbManager::instance().getDatabase());
    QString sql = "SELECT full_name, short_name, contact_phone, contact_email FROM tv.channels WHERE city ILIKE '%" + city + "%' ORDER BY full_name";
    if (!query.exec(sql)) {
        QMessageBox::critical(this, "Ошибка", query.lastError().text());
        return;
    }
    QString result;
    while (query.next()) {
        result += QString("%1 (%2) | Тел: %3 | Email: %4\n")
                  .arg(query.value(0).toString(), query.value(1).toString(),
                       query.value(2).toString(), query.value(3).toString());
    }
    showReport("Телеканалы в городе " + city, result.isEmpty() ? "Нет данных" : result);
}

void ReportsTab::reportChannelWithBroadcasts() {
    QSqlQuery q(DbManager::instance().getDatabase());
    q.exec("SELECT MIN(channel_id) as channel_id, full_name FROM tv.channels GROUP BY full_name ORDER BY full_name");
    QStringList items;
    QList<int> ids;
    while (q.next()) {
        items << q.value(1).toString();
        ids << q.value(0).toInt();
    }
    if (items.isEmpty()) {
        QMessageBox::information(this, "Информация", "Нет зарегистрированных каналов");
        return;
    }
    bool ok;
    QString selected = getItemOneClick("Выбор канала", "Выберите канал:", items, 0, &ok);
    if (!ok || selected.isEmpty()) return;
    int idx = items.indexOf(selected);
    int channelId = ids[idx];

    QSqlQuery query(DbManager::instance().getDatabase());
    QString sql = "SELECT c.full_name, b.title, b.air_date_time, b.rating "
                  "FROM tv.channels c LEFT JOIN tv.broadcasts b ON c.channel_id = b.channel_id "
                  "WHERE c.channel_id = " + QString::number(channelId) + " ORDER BY b.air_date_time";
    if (!query.exec(sql)) {
        QMessageBox::critical(this, "Ошибка", query.lastError().text());
        return;
    }
    QString result;
    QString channelName;
    while (query.next()) {
        if (channelName.isEmpty()) channelName = query.value(0).toString();
        QString title = query.value(1).toString();
        if (title.isEmpty()) title = "(нет передач)";
        result += QString("Передача: %1 | Дата: %2 | Рейтинг: %3\n")
                  .arg(title, query.value(2).toString(), query.value(3).toString());
    }
    showReport("Канал: " + channelName, result.isEmpty() ? "Нет передач" : result);
}

void ReportsTab::reportViewersByStatus() {
    QStringList statuses = {"active", "vip", "blocked"};
    bool ok;
    QString status = getItemOneClick("Статус", "Выберите статус:", statuses, 0, &ok);
    if (!ok || status.isEmpty()) return;
    QSqlQuery query(DbManager::instance().getDatabase());
    QString sql = "SELECT last_name, first_name, phone, email FROM tv.users WHERE subscription_status = '" + status + "' ORDER BY last_name";
    if (!query.exec(sql)) {
        QMessageBox::critical(this, "Ошибка", query.lastError().text());
        return;
    }
    QString result;
    while (query.next()) {
        result += QString("%1 %2 | Тел: %3 | Email: %4\n")
                  .arg(query.value(0).toString(), query.value(1).toString(),
                       query.value(2).toString(), query.value(3).toString());
    }
    showReport("Зрители со статусом '" + status + "'", result.isEmpty() ? "Нет данных" : result);
}

void ReportsTab::reportViewerHistory() {
    // Если передан конкретный userId (зритель) – используем его, иначе выбор
    int viewerId = currentUserId;
    QString selectedName;
    if (viewerId <= 0) {
        QSqlQuery q(DbManager::instance().getDatabase());
        q.exec("SELECT user_id, last_name || ' ' || first_name FROM tv.users ORDER BY last_name");
        QStringList items;
        QList<int> ids;
        while (q.next()) {
            items << q.value(1).toString();
            ids << q.value(0).toInt();
        }
        if (items.isEmpty()) {
            QMessageBox::information(this, "Информация", "Нет зарегистрированных зрителей");
            return;
        }
        bool ok;
        selectedName = getItemOneClick("Выбор зрителя", "Выберите зрителя:", items, 0, &ok);
        if (!ok || selectedName.isEmpty()) return;
        int idx = items.indexOf(selectedName);
        viewerId = ids[idx];
        selectedName = items[idx];
    } else {
        // Получим имя текущего зрителя для заголовка
        QSqlQuery q(DbManager::instance().getDatabase());
        q.prepare("SELECT last_name || ' ' || first_name FROM tv.users WHERE user_id = ?");
        q.bindValue(0, viewerId);
        if (q.exec() && q.next()) selectedName = q.value(0).toString();
    }

    QSqlQuery query(DbManager::instance().getDatabase());
    query.prepare("SELECT b.title, vh.view_date, vh.watch_duration_minutes "
                  "FROM tv.view_history vh JOIN tv.broadcasts b ON vh.broadcast_id = b.broadcast_id "
                  "WHERE vh.user_id = ? ORDER BY vh.view_date DESC");
    query.bindValue(0, viewerId);
    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка", query.lastError().text());
        return;
    }
    QString result;
    while (query.next()) {
        result += QString("%1 | Дата: %2 | Длит: %3 мин\n")
                  .arg(query.value(0).toString(), query.value(1).toString(), query.value(2).toString());
    }
    showReport("История просмотров зрителя " + selectedName, result.isEmpty() ? "Нет просмотров" : result);
}

void ReportsTab::reportBroadcastViewers() {
    QSqlQuery q(DbManager::instance().getDatabase());
    q.exec("SELECT MIN(broadcast_id) as broadcast_id, title FROM tv.broadcasts GROUP BY title ORDER BY title");
    QStringList items;
    QList<int> ids;
    while (q.next()) {
        items << q.value(1).toString();
        ids << q.value(0).toInt();
    }
    if (items.isEmpty()) {
        QMessageBox::information(this, "Информация", "Нет зарегистрированных передач");
        return;
    }
    bool ok;
    QString selected = getItemOneClick("Выбор передачи", "Выберите передачу:", items, 0, &ok);
    if (!ok || selected.isEmpty()) return;
    int idx = items.indexOf(selected);
    int broadcastId = ids[idx];

    QSqlQuery query(DbManager::instance().getDatabase());
    query.prepare("SELECT u.last_name, u.first_name, vh.view_date "
                  "FROM tv.view_history vh JOIN tv.users u ON vh.user_id = u.user_id "
                  "WHERE vh.broadcast_id = ? ORDER BY vh.view_date");
    query.bindValue(0, broadcastId);
    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка", query.lastError().text());
        return;
    }
    QString result;
    while (query.next()) {
        result += QString("%1 %2 | Дата просмотра: %3\n")
                  .arg(query.value(0).toString(), query.value(1).toString(), query.value(2).toString());
    }
    showReport("Зрители передачи «" + selected + "»", result.isEmpty() ? "Нет зрителей" : result);
}

void ReportsTab::reportChannelBroadcastsViewers() {
    QSqlQuery q(DbManager::instance().getDatabase());
    q.exec("SELECT MIN(channel_id) as channel_id, full_name FROM tv.channels GROUP BY full_name ORDER BY full_name");
    QStringList items;
    QList<int> ids;
    while (q.next()) {
        items << q.value(1).toString();
        ids << q.value(0).toInt();
    }
    if (items.isEmpty()) {
        QMessageBox::information(this, "Информация", "Нет зарегистрированных каналов");
        return;
    }
    bool ok;
    QString selected = getItemOneClick("Выбор канала", "Выберите канал:", items, 0, &ok);
    if (!ok || selected.isEmpty()) return;
    int idx = items.indexOf(selected);
    int channelId = ids[idx];

    QSqlQuery query(DbManager::instance().getDatabase());
    query.prepare("SELECT broadcast_title, air_date_time, last_name, first_name, view_date "
                  "FROM tv.channel_broadcasts_viewers WHERE channel_id = ? ORDER BY air_date_time, view_date");
    query.bindValue(0, channelId);
    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка", query.lastError().text());
        return;
    }
    QString result;
    while (query.next()) {
        result += QString("Передача: %1 | Эфир: %2 | Зритель: %3 %4 | Просмотр: %5\n")
                  .arg(query.value(0).toString(), query.value(1).toString(),
                       query.value(2).toString(), query.value(3).toString(), query.value(4).toString());
    }
    showReport("Передачи канала «" + selected + "» со зрителями", result.isEmpty() ? "Нет данных" : result);
}

void ReportsTab::reportChannelViewers() {
    QSqlQuery q(DbManager::instance().getDatabase());
    q.exec("SELECT MIN(channel_id) as channel_id, full_name FROM tv.channels GROUP BY full_name ORDER BY full_name");
    QStringList items;
    QList<int> ids;
    while (q.next()) {
        items << q.value(1).toString();
        ids << q.value(0).toInt();
    }
    if (items.isEmpty()) {
        QMessageBox::information(this, "Информация", "Нет зарегистрированных каналов");
        return;
    }
    bool ok;
    QString selected = getItemOneClick("Выбор канала", "Выберите канал:", items, 0, &ok);
    if (!ok || selected.isEmpty()) return;
    int idx = items.indexOf(selected);
    int channelId = ids[idx];

    QSqlQuery query(DbManager::instance().getDatabase());
    query.prepare("SELECT DISTINCT u.last_name, u.first_name, u.phone, u.email "
                  "FROM tv.channel_broadcasts_viewers cbv JOIN tv.users u ON cbv.user_id = u.user_id "
                  "WHERE cbv.channel_id = ? ORDER BY u.last_name");
    query.bindValue(0, channelId);
    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка", query.lastError().text());
        return;
    }
    QString result;
    while (query.next()) {
        result += QString("%1 %2 | Тел: %3 | Email: %4\n")
                  .arg(query.value(0).toString(), query.value(1).toString(),
                       query.value(2).toString(), query.value(3).toString());
    }
    showReport("Зрители канала «" + selected + "»", result.isEmpty() ? "Нет зрителей" : result);
}

void ReportsTab::reportMyBroadcasts() {
    if (currentUserId > 0) {
        // Зритель – свои просмотры
        QSqlQuery query(DbManager::instance().getDatabase());
        query.prepare("SELECT b.title, vh.view_date, vh.watch_duration_minutes "
                      "FROM tv.view_history vh JOIN tv.broadcasts b ON vh.broadcast_id = b.broadcast_id "
                      "WHERE vh.user_id = ? ORDER BY vh.view_date DESC");
        query.bindValue(0, currentUserId);
        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", query.lastError().text());
            return;
        }
        QString result;
        while (query.next()) {
            result += QString("%1 | Дата: %2 | Длит: %3 мин\n")
                      .arg(query.value(0).toString(), query.value(1).toString(), query.value(2).toString());
        }
        showReport("Мои просмотры", result.isEmpty() ? "Нет просмотров" : result);
    } else {
        // Администратор, сотрудник канала, надзор – выбирают зрителя
        QSqlQuery q(DbManager::instance().getDatabase());
        q.exec("SELECT user_id, last_name || ' ' || first_name FROM tv.users ORDER BY last_name");
        QStringList items;
        QList<int> ids;
        while (q.next()) {
            items << q.value(1).toString();
            ids << q.value(0).toInt();
        }
        if (items.isEmpty()) {
            QMessageBox::information(this, "Информация", "Нет зарегистрированных зрителей");
            return;
        }
        bool ok;
        QString selected = getItemOneClick("Выбор зрителя", "Выберите зрителя:", items, 0, &ok);
        if (!ok || selected.isEmpty()) return;
        int idx = items.indexOf(selected);
        int viewerId = ids[idx];
        QSqlQuery query(DbManager::instance().getDatabase());
        query.prepare("SELECT b.title, vh.view_date, vh.watch_duration_minutes "
                      "FROM tv.view_history vh JOIN tv.broadcasts b ON vh.broadcast_id = b.broadcast_id "
                      "WHERE vh.user_id = ? ORDER BY vh.view_date DESC");
        query.bindValue(0, viewerId);
        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", query.lastError().text());
            return;
        }
        QString result;
        while (query.next()) {
            result += QString("%1 | Дата: %2 | Длит: %3 мин\n")
                      .arg(query.value(0).toString(), query.value(1).toString(), query.value(2).toString());
        }
        showReport("Просмотры зрителя " + selected, result.isEmpty() ? "Нет просмотров" : result);
    }
}

void ReportsTab::reportMyAccount() {
    if (currentUserId > 0) {
        // Зритель – свои операции
        QSqlQuery query(DbManager::instance().getDatabase());
        query.prepare("SELECT change_date, old_status, new_status, comment FROM tv.subscription_history WHERE user_id = ? ORDER BY change_date DESC");
        query.bindValue(0, currentUserId);
        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", query.lastError().text());
            return;
        }
        QString result;
        while (query.next()) {
            result += QString("%1 | %2 → %3 | %4\n")
                      .arg(query.value(0).toString(), query.value(1).toString(),
                           query.value(2).toString(), query.value(3).toString());
        }
        showReport("История операций по лицевому счёту", result.isEmpty() ? "Нет операций" : result);
    } else {
        // Другие роли – выбор зрителя
        QSqlQuery q(DbManager::instance().getDatabase());
        q.exec("SELECT user_id, last_name || ' ' || first_name FROM tv.users ORDER BY last_name");
        QStringList items;
        QList<int> ids;
        while (q.next()) {
            items << q.value(1).toString();
            ids << q.value(0).toInt();
        }
        if (items.isEmpty()) {
            QMessageBox::information(this, "Информация", "Нет зарегистрированных зрителей");
            return;
        }
        bool ok;
        QString selected = getItemOneClick("Выбор зрителя", "Выберите зрителя:", items, 0, &ok);
        if (!ok || selected.isEmpty()) return;
        int idx = items.indexOf(selected);
        int viewerId = ids[idx];
        QSqlQuery query(DbManager::instance().getDatabase());
        query.prepare("SELECT change_date, old_status, new_status, comment FROM tv.subscription_history WHERE user_id = ? ORDER BY change_date DESC");
        query.bindValue(0, viewerId);
        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", query.lastError().text());
            return;
        }
        QString result;
        while (query.next()) {
            result += QString("%1 | %2 → %3 | %4\n")
                      .arg(query.value(0).toString(), query.value(1).toString(),
                           query.value(2).toString(), query.value(3).toString());
        }
        showReport("Операции по лицевому счёту зрителя " + selected, result.isEmpty() ? "Нет операций" : result);
    }
}