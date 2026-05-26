#ifndef BROADCAST_TAB_H
#define BROADCAST_TAB_H

#include <QWidget>

class QTableWidget;
class QLineEdit;
class QComboBox;
class QDateTimeEdit;
class QPushButton;

class BroadcastTab : public QWidget {
    Q_OBJECT
public:
    explicit BroadcastTab(QWidget *parent = nullptr);
    ~BroadcastTab();

private slots:
    void refresh();
    void onAdd();
    void onSave();
    void onDelete();
    void onSearchById();
    void onSearchByTitle();
    void onRowSelected(int row, int col);

private:
    QTableWidget *table;
    QLineEdit *leTitle, *leDuration, *leRating, *leSearchId, *leSearchTitle;
    QComboBox *cbGenre, *cbChannel;
    QDateTimeEdit *dtAir;
    QPushButton *btnAdd, *btnSave, *btnDelete, *btnRefresh, *btnSearchId, *btnSearchTitle;
    int currentBroadcastId;
    void loadBroadcasts(int searchId = 0, const QString& searchTitle = "");
    void clearForm();
    void setFormEnabled(bool en);
    void loadBroadcastToForm(int id);
};

#endif // BROADCAST_TAB_H
