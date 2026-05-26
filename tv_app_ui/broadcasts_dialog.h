#ifndef BROADCASTS_DIALOG_H
#define BROADCASTS_DIALOG_H

#include <QDialog>

namespace Ui {
class BroadcastsDialog;
}

class BroadcastsDialog : public QDialog {
    Q_OBJECT
public:
    explicit BroadcastsDialog(int channelId, QWidget *parent = nullptr);
    ~BroadcastsDialog();

private:
    Ui::BroadcastsDialog *ui;
    int channelId;
    void loadBroadcasts();
};

#endif // BROADCASTS_DIALOG_H
