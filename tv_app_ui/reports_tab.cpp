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

// Кастомный комбобокс для одного тапа
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

ReportsTab::ReportsTab(int viewerId, QWidget *parent) : QWidget(parent), currentViewerId(viewerId) {
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

void ReportsTab::reportChannelsByCity() { /* без изменений (был ранее) */ }
void ReportsTab::reportChannelWithBroadcasts() { /* без изменений */ }
void ReportsTab::reportViewersByStatus() { /* без изменений */ }
void ReportsTab::reportViewerHistory() { /* без изменений */ }
void ReportsTab::reportBroadcastViewers() { /* без изменений */ }
void ReportsTab::reportChannelBroadcastsViewers() { /* без изменений */ }
void ReportsTab::reportChannelViewers() { /* без изменений */ }

void ReportsTab::reportMyBroadcasts() {
    if (currentViewerId > 0) {
        // Зритель автоматически получает свои просмотры
        QSqlQuery query(DbManager::instance().getDatabase());
        query.prepare("SELECT b.title, vh.view_date, vh.watch_duration_minutes "
                      "FROM tv.view_history vh JOIN tv.broadcasts b ON vh.broadcast_id = b.broadcast_id "
                      "WHERE vh.viewer_id = ? ORDER BY vh.view_date DESC");
        query.bindValue(0, currentViewerId);
        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", query.lastError().text());
            return;
        }
        QString result;
        while (query.next()) {
            result += QString("%1 | %2 | %3 мин\n").arg(query.value(0).toString(), query.value(1).toString(), query.value(2).toString());
        }
        showReport("Мои просмотры", result.isEmpty() ? "Нет просмотров" : result);
    } else {
        // Администратор, сотрудник канала, надзор – выбирают зрителя
        QSqlQuery q(DbManager::instance().getDatabase());
        q.exec("SELECT MIN(viewer_id) as viewer_id, last_name || ' ' || first_name as full_name FROM tv.viewers GROUP BY last_name, first_name ORDER BY last_name");
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
                      "WHERE vh.viewer_id = ? ORDER BY vh.view_date DESC");
        query.bindValue(0, viewerId);
        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", query.lastError().text());
            return;
        }
        QString result;
        while (query.next()) {
            result += QString("%1 | %2 | %3 мин\n").arg(query.value(0).toString(), query.value(1).toString(), query.value(2).toString());
        }
        showReport("Просмотры зрителя " + selected, result.isEmpty() ? "Нет просмотров" : result);
    }
}

void ReportsTab::reportMyAccount() {
    if (currentViewerId > 0) {
        // Зритель автоматически получает свои операции
        QSqlQuery query(DbManager::instance().getDatabase());
        query.prepare("SELECT change_date, old_status, new_status, comment FROM tv.subscription_history WHERE viewer_id = ? ORDER BY change_date DESC");
        query.bindValue(0, currentViewerId);
        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", query.lastError().text());
            return;
        }
        QString result;
        while (query.next()) {
            result += QString("%1 | %2 → %3 | %4\n").arg(query.value(0).toString(), query.value(1).toString(), query.value(2).toString(), query.value(3).toString());
        }
        showReport("История операций по лицевому счёту", result.isEmpty() ? "Нет операций" : result);
    } else {
        // Другие роли – выбирают зрителя
        QSqlQuery q(DbManager::instance().getDatabase());
        q.exec("SELECT MIN(viewer_id) as viewer_id, last_name || ' ' || first_name as full_name FROM tv.viewers GROUP BY last_name, first_name ORDER BY last_name");
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
        showReport("Операции по лицевому счёту зрителя " + selected, result.isEmpty() ? "Нет операций" : result);
    }
}