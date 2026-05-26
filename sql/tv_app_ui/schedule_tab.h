#ifndef SCHEDULE_TAB_H
#define SCHEDULE_TAB_H

#include <QWidget>
#include <QComboBox>
#include <QMouseEvent>

// Комбобокс, открывающийся по одному клику (не двойному)
class OneClickComboBox : public QComboBox {
    Q_OBJECT
public:
    using QComboBox::QComboBox;
    void mousePressEvent(QMouseEvent *e) override {
        if (e->button() == Qt::LeftButton) {
            showPopup();
        } else {
            QComboBox::mousePressEvent(e);
        }
    }
};

class QTableWidget;
class QDateTimeEdit;
class QPushButton;

class ScheduleTab : public QWidget {
    Q_OBJECT
public:
    explicit ScheduleTab(QWidget *parent = nullptr);
    ~ScheduleTab();

private slots:
    void refresh();

private:
    QTableWidget *table;
    QDateTimeEdit *dtFrom;
    QDateTimeEdit *dtTo;
    OneClickComboBox *cbChannel;
    QPushButton *btnRefresh;
    void loadSchedule(const QString& from, const QString& to, int channelId);
};

#endif // SCHEDULE_TAB_H
