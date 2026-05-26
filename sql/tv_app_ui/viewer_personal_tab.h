#ifndef VIEWER_PERSONAL_TAB_H
#define VIEWER_PERSONAL_TAB_H

#include <QWidget>

class QLineEdit;
class QDateEdit;
class QComboBox;
class QTableWidget;
class QPushButton;

class ViewerPersonalTab : public QWidget {
    Q_OBJECT
public:
    explicit ViewerPersonalTab(int userId, QWidget *parent = nullptr);
    ~ViewerPersonalTab();

private slots:
    void saveInfo();
    void searchViews();
    void reportAllViews();
    void reportAccountOperations();

private:
    int userId;
    int viewerId;
    QLineEdit *leLastName, *leFirstName, *lePassport, *lePhone, *leEmail;
    QDateEdit *deBirth;
    QComboBox *cbStatus;
    QLineEdit *leSearchDateFrom, *leSearchDateTo, *leSearchStatus;
    QTableWidget *table;
    void loadViewerData();
    void loadViews(const QString& dateFrom, const QString& dateTo, const QString& status);
};

#endif // VIEWER_PERSONAL_TAB_H
