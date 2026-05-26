#ifndef AUTH_DIALOG_H
#define AUTH_DIALOG_H

#include <QDialog>

namespace Ui {
class AuthDialog;
}

class AuthDialog : public QDialog {
    Q_OBJECT
public:
    explicit AuthDialog(QWidget *parent = nullptr);
    ~AuthDialog();
    int getUserId() const;
    QString getUserRole() const;
    QString getUserName() const;

private slots:
    void onLogin();

private:
    Ui::AuthDialog *ui;
    int userId;
    QString userRole;
    QString username;
};

#endif // AUTH_DIALOG_H