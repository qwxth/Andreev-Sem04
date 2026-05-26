#ifndef REPORTS_TAB_H
#define REPORTS_TAB_H

#include <QWidget>

class QTextEdit;

class ReportsTab : public QWidget {
    Q_OBJECT
public:
    // добавим параметр viewerId (по умолчанию 0) для персональных отчётов зрителя
    explicit ReportsTab(int viewerId = 0, QWidget *parent = nullptr);

private slots:
    void reportChannelsByCity();
    void reportChannelWithBroadcasts();
    void reportViewersByStatus();
    void reportViewerHistory();
    void reportBroadcastViewers();
    void reportChannelBroadcastsViewers();
    void reportChannelViewers();
    void reportMyBroadcasts();
    void reportMyAccount();

private:
    QTextEdit *textEdit;
    int currentViewerId; // для зрителя
    void showReport(const QString& title, const QString& content);
};

#endif // REPORTS_TAB_H