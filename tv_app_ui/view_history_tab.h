#ifndef VIEW_HISTORY_TAB_H
#define VIEW_HISTORY_TAB_H

#include <QWidget>

class QTableWidget;
class QComboBox;
class QLineEdit;
class QPushButton;

class ViewHistoryTab : public QWidget {
    Q_OBJECT
public:
    explicit ViewHistoryTab(QWidget *parent = nullptr);
    ~ViewHistoryTab();

private slots:
    void refresh();
    void onSearch();
    void onAdd();
    void onDelete();
    void onRowSelected(int row, int col);

private:
    QTableWidget *table;
    QComboBox *cbViewer;
    QLineEdit *leViewerId;
    QPushButton *btnAdd, *btnDelete, *btnRefresh, *btnSearch;
    int currentViewId;
    void loadHistory(int viewerId = 0);
    void clearForm();
};

#endif // VIEW_HISTORY_TAB_H
