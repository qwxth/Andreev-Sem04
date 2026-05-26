/********************************************************************************
** Form generated from reading UI file 'auth_dialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.18
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_AUTH_DIALOG_H
#define UI_AUTH_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AuthDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QLineEdit *leUsername;
    QLineEdit *lePassword;
    QPushButton *btnLogin;

    void setupUi(QDialog *AuthDialog)
    {
        if (AuthDialog->objectName().isEmpty())
            AuthDialog->setObjectName(QString::fromUtf8("AuthDialog"));
        AuthDialog->resize(300, 180);
        verticalLayout = new QVBoxLayout(AuthDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(AuthDialog);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        leUsername = new QLineEdit(AuthDialog);
        leUsername->setObjectName(QString::fromUtf8("leUsername"));

        verticalLayout->addWidget(leUsername);

        lePassword = new QLineEdit(AuthDialog);
        lePassword->setObjectName(QString::fromUtf8("lePassword"));
        lePassword->setEchoMode(QLineEdit::Password);

        verticalLayout->addWidget(lePassword);

        btnLogin = new QPushButton(AuthDialog);
        btnLogin->setObjectName(QString::fromUtf8("btnLogin"));

        verticalLayout->addWidget(btnLogin);


        retranslateUi(AuthDialog);

        QMetaObject::connectSlotsByName(AuthDialog);
    } // setupUi

    void retranslateUi(QDialog *AuthDialog)
    {
        AuthDialog->setWindowTitle(QCoreApplication::translate("AuthDialog", "\320\220\320\262\321\202\320\276\321\200\320\270\320\267\320\260\321\206\320\270\321\217", nullptr));
        label->setText(QCoreApplication::translate("AuthDialog", "\320\222\320\262\320\265\320\264\320\270\321\202\320\265 \320\273\320\276\320\263\320\270\320\275 \320\270 \320\277\320\260\321\200\320\276\320\273\321\214:", nullptr));
        leUsername->setPlaceholderText(QCoreApplication::translate("AuthDialog", "\320\233\320\276\320\263\320\270\320\275", nullptr));
        lePassword->setPlaceholderText(QCoreApplication::translate("AuthDialog", "\320\237\320\260\321\200\320\276\320\273\321\214", nullptr));
        btnLogin->setText(QCoreApplication::translate("AuthDialog", "\320\222\320\276\320\271\321\202\320\270", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AuthDialog: public Ui_AuthDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AUTH_DIALOG_H
