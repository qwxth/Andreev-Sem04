#ifndef VIEWER_TAB_H
#define VIEWER_TAB_H

#include <QWidget>

class QTableWidget;
class QLineEdit;
class QDateEdit;
class QComboBox;
class QPushButton;

class ViewerTab : public QWidget {
    Q_OBJECT
public:
    explicit ViewerTab(QWidget *parent = nullptr);
    ~ViewerTab();

private slots:
    void refresh();
    void onSearch();
    void onAdd();
    void onSave();
    void onDelete();
    void onRowSelected(int row, int col);

private:
    QTableWidget *table;
    QLineEdit *leSearch, *leLastName, *leFirstName, *lePassport, *lePhone, *leEmail;
    QDateEdit *deBirth;
    QComboBox *cbStatus;
    QPushButton *btnAdd, *btnSave, *btnDelete, *btnRefresh, *btnSearch;
    int currentUserId;
    void loadUsers(const QString& filter = "");
    void clearForm();
    void setFormEnabled(bool en);
    void loadUserToForm(int id);
};

#endif // VIEWER_TAB_H