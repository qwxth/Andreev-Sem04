/********************************************************************************
** Form generated from reading UI file 'admin_tab.ui'
**
** Created by: Qt User Interface Compiler version 5.15.18
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ADMIN_TAB_H
#define UI_ADMIN_TAB_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AdminTab
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBoxChannels;
    QVBoxLayout *verticalLayout_2;
    QTableWidget *tableChannels;
    QGroupBox *groupBoxEdit;
    QGridLayout *gridLayout;
    QLabel *labelFullName;
    QLineEdit *leFullName;
    QLabel *labelShortName;
    QLineEdit *leShortName;
    QLabel *labelCountry;
    QLineEdit *leCountry;
    QLabel *labelCity;
    QLineEdit *leCity;
    QLabel *labelOwner;
    QLineEdit *leOwner;
    QLabel *labelPhone;
    QLineEdit *lePhone;
    QLabel *labelEmail;
    QLineEdit *leEmail;
    QHBoxLayout *horizontalLayout;
    QPushButton *btnAdd;
    QPushButton *btnSave;
    QPushButton *btnDelete;
    QPushButton *btnRefresh;
    QGroupBox *groupBoxBroadcasts;
    QVBoxLayout *verticalLayout_3;
    QTableWidget *tableBroadcasts;

    void setupUi(QWidget *AdminTab)
    {
        if (AdminTab->objectName().isEmpty())
            AdminTab->setObjectName(QString::fromUtf8("AdminTab"));
        verticalLayout = new QVBoxLayout(AdminTab);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBoxChannels = new QGroupBox(AdminTab);
        groupBoxChannels->setObjectName(QString::fromUtf8("groupBoxChannels"));
        verticalLayout_2 = new QVBoxLayout(groupBoxChannels);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        tableChannels = new QTableWidget(groupBoxChannels);
        tableChannels->setObjectName(QString::fromUtf8("tableChannels"));

        verticalLayout_2->addWidget(tableChannels);


        verticalLayout->addWidget(groupBoxChannels);

        groupBoxEdit = new QGroupBox(AdminTab);
        groupBoxEdit->setObjectName(QString::fromUtf8("groupBoxEdit"));
        gridLayout = new QGridLayout(groupBoxEdit);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        labelFullName = new QLabel(groupBoxEdit);
        labelFullName->setObjectName(QString::fromUtf8("labelFullName"));

        gridLayout->addWidget(labelFullName, 0, 0, 1, 1);

        leFullName = new QLineEdit(groupBoxEdit);
        leFullName->setObjectName(QString::fromUtf8("leFullName"));

        gridLayout->addWidget(leFullName, 0, 1, 1, 1);

        labelShortName = new QLabel(groupBoxEdit);
        labelShortName->setObjectName(QString::fromUtf8("labelShortName"));

        gridLayout->addWidget(labelShortName, 1, 0, 1, 1);

        leShortName = new QLineEdit(groupBoxEdit);
        leShortName->setObjectName(QString::fromUtf8("leShortName"));

        gridLayout->addWidget(leShortName, 1, 1, 1, 1);

        labelCountry = new QLabel(groupBoxEdit);
        labelCountry->setObjectName(QString::fromUtf8("labelCountry"));

        gridLayout->addWidget(labelCountry, 2, 0, 1, 1);

        leCountry = new QLineEdit(groupBoxEdit);
        leCountry->setObjectName(QString::fromUtf8("leCountry"));

        gridLayout->addWidget(leCountry, 2, 1, 1, 1);

        labelCity = new QLabel(groupBoxEdit);
        labelCity->setObjectName(QString::fromUtf8("labelCity"));

        gridLayout->addWidget(labelCity, 3, 0, 1, 1);

        leCity = new QLineEdit(groupBoxEdit);
        leCity->setObjectName(QString::fromUtf8("leCity"));

        gridLayout->addWidget(leCity, 3, 1, 1, 1);

        labelOwner = new QLabel(groupBoxEdit);
        labelOwner->setObjectName(QString::fromUtf8("labelOwner"));

        gridLayout->addWidget(labelOwner, 4, 0, 1, 1);

        leOwner = new QLineEdit(groupBoxEdit);
        leOwner->setObjectName(QString::fromUtf8("leOwner"));

        gridLayout->addWidget(leOwner, 4, 1, 1, 1);

        labelPhone = new QLabel(groupBoxEdit);
        labelPhone->setObjectName(QString::fromUtf8("labelPhone"));

        gridLayout->addWidget(labelPhone, 5, 0, 1, 1);

        lePhone = new QLineEdit(groupBoxEdit);
        lePhone->setObjectName(QString::fromUtf8("lePhone"));

        gridLayout->addWidget(lePhone, 5, 1, 1, 1);

        labelEmail = new QLabel(groupBoxEdit);
        labelEmail->setObjectName(QString::fromUtf8("labelEmail"));

        gridLayout->addWidget(labelEmail, 6, 0, 1, 1);

        leEmail = new QLineEdit(groupBoxEdit);
        leEmail->setObjectName(QString::fromUtf8("leEmail"));

        gridLayout->addWidget(leEmail, 6, 1, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        btnAdd = new QPushButton(groupBoxEdit);
        btnAdd->setObjectName(QString::fromUtf8("btnAdd"));

        horizontalLayout->addWidget(btnAdd);

        btnSave = new QPushButton(groupBoxEdit);
        btnSave->setObjectName(QString::fromUtf8("btnSave"));

        horizontalLayout->addWidget(btnSave);

        btnDelete = new QPushButton(groupBoxEdit);
        btnDelete->setObjectName(QString::fromUtf8("btnDelete"));

        horizontalLayout->addWidget(btnDelete);

        btnRefresh = new QPushButton(groupBoxEdit);
        btnRefresh->setObjectName(QString::fromUtf8("btnRefresh"));

        horizontalLayout->addWidget(btnRefresh);


        gridLayout->addLayout(horizontalLayout, 7, 0, 1, 2);


        verticalLayout->addWidget(groupBoxEdit);

        groupBoxBroadcasts = new QGroupBox(AdminTab);
        groupBoxBroadcasts->setObjectName(QString::fromUtf8("groupBoxBroadcasts"));
        verticalLayout_3 = new QVBoxLayout(groupBoxBroadcasts);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        tableBroadcasts = new QTableWidget(groupBoxBroadcasts);
        tableBroadcasts->setObjectName(QString::fromUtf8("tableBroadcasts"));

        verticalLayout_3->addWidget(tableBroadcasts);


        verticalLayout->addWidget(groupBoxBroadcasts);


        retranslateUi(AdminTab);

        QMetaObject::connectSlotsByName(AdminTab);
    } // setupUi

    void retranslateUi(QWidget *AdminTab)
    {
        groupBoxChannels->setTitle(QCoreApplication::translate("AdminTab", "\320\242\320\265\320\273\320\265\320\272\320\260\320\275\320\260\320\273\321\213", nullptr));
        groupBoxEdit->setTitle(QCoreApplication::translate("AdminTab", "\320\240\320\265\320\264\320\260\320\272\321\202\320\270\321\200\320\276\320\262\320\260\320\275\320\270\320\265 \320\272\320\260\320\275\320\260\320\273\320\260", nullptr));
        labelFullName->setText(QCoreApplication::translate("AdminTab", "\320\237\320\276\320\273\320\275\320\276\320\265 \320\275\320\260\320\267\320\262\320\260\320\275\320\270\320\265:", nullptr));
        labelShortName->setText(QCoreApplication::translate("AdminTab", "\320\232\321\200\320\260\321\202\320\272\320\276\320\265 \320\275\320\260\320\267\320\262\320\260\320\275\320\270\320\265:", nullptr));
        labelCountry->setText(QCoreApplication::translate("AdminTab", "\320\241\321\202\321\200\320\260\320\275\320\260:", nullptr));
        labelCity->setText(QCoreApplication::translate("AdminTab", "\320\223\320\276\321\200\320\276\320\264:", nullptr));
        labelOwner->setText(QCoreApplication::translate("AdminTab", "\320\222\320\273\320\260\320\264\320\265\320\273\320\265\321\206:", nullptr));
        labelPhone->setText(QCoreApplication::translate("AdminTab", "\320\242\320\265\320\273\320\265\321\204\320\276\320\275:", nullptr));
        labelEmail->setText(QCoreApplication::translate("AdminTab", "Email:", nullptr));
        btnAdd->setText(QCoreApplication::translate("AdminTab", "\320\224\320\276\320\261\320\260\320\262\320\270\321\202\321\214", nullptr));
        btnSave->setText(QCoreApplication::translate("AdminTab", "\320\241\320\276\321\205\321\200\320\260\320\275\320\270\321\202\321\214", nullptr));
        btnDelete->setText(QCoreApplication::translate("AdminTab", "\320\243\320\264\320\260\320\273\320\270\321\202\321\214", nullptr));
        btnRefresh->setText(QCoreApplication::translate("AdminTab", "\320\236\320\261\320\275\320\276\320\262\320\270\321\202\321\214", nullptr));
        groupBoxBroadcasts->setTitle(QCoreApplication::translate("AdminTab", "\320\237\320\265\321\200\320\265\320\264\320\260\321\207\320\270 \320\262\321\213\320\261\321\200\320\260\320\275\320\275\320\276\320\263\320\276 \320\272\320\260\320\275\320\260\320\273\320\260", nullptr));
        (void)AdminTab;
    } // retranslateUi

};

namespace Ui {
    class AdminTab: public Ui_AdminTab {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADMIN_TAB_H
