#include <QApplication>
#include <QMessageBox>
#include "db_manager.h"
#include "auth_dialog.h"
#include "main_window.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    if (!DbManager::instance().connect()) {
        QMessageBox::critical(nullptr, "Ошибка", "Не удалось подключиться к базе данных");
        return -1;
    }

    AuthDialog auth;
    if (auth.exec() == QDialog::Accepted) {
        MainWindow w(auth.getUserRole(), auth.getUserId(), auth.getUserName());
        w.show();
        return a.exec();
    }
    return 0;
}
