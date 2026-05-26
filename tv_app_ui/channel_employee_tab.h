#ifndef CHANNEL_EMPLOYEE_TAB_H
#define CHANNEL_EMPLOYEE_TAB_H

#include <QWidget>

class QTableWidget;
class QLineEdit;
class QDateTimeEdit;
class QPushButton;

class ChannelEmployeeTab : public QWidget {
    Q_OBJECT
public:
    explicit ChannelEmployeeTab(int userId, QWidget *parent = nullptr);

private slots:
    void saveChannel();
    void searchBroadcasts();
    void reportBroadcastsWithViewers();
    void reportViewersOfChannel();

private:
    int userId;
    int channelId;
    QLineEdit *leFullName, *leShortName, *leCountry, *leCity, *leOwner, *lePhone, *leEmail;
    QLineEdit *leBroadcastId, *leRatingFrom, *leRatingTo;
    QDateTimeEdit *leDateFrom, *leDateTo;
    QTableWidget *table;
    void loadChannelData();
    void loadBroadcasts(const QString& broadcastId, const QDateTime& from, const QDateTime& to, double ratingFrom, double ratingTo);
};

#endif // CHANNEL_EMPLOYEE_TAB_H
