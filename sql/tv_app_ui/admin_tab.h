#ifndef ADMIN_TAB_H
#define ADMIN_TAB_H

#include <QWidget>

namespace Ui {
class AdminTab;
}

class AdminTab : public QWidget {
    Q_OBJECT
public:
    explicit AdminTab(QWidget *parent = nullptr);
    ~AdminTab();

private slots:
    void refreshAll();
    void onTableRowClicked(int row, int column);
    void onAdd();
    void onSave();
    void onDelete();

private:
    Ui::AdminTab *ui;
    int currentChannelId;
    void loadChannels();
    void loadBroadcasts(int channelId);
    void clearForm();
    void setFormEnabled(bool enabled);
    void loadChannelToForm(int channelId);
};

#endif // ADMIN_TAB_H
