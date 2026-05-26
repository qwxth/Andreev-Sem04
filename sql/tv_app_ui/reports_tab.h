#ifndef REPORTS_TAB_H
#define REPORTS_TAB_H

#include <QWidget>

class QTextEdit;

class ReportsTab : public QWidget {
    Q_OBJECT
public:
    explicit ReportsTab(int currentUserId = 0, QWidget *parent = nullptr);

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
    int currentUserId;
    void showReport(const QString& title, const QString& content);
};

#endif // REPORTS_TAB_H