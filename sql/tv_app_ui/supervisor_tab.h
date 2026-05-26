#ifndef SUPERVISOR_TAB_H
#define SUPERVISOR_TAB_H

#include <QWidget>

class QTabWidget;

class SupervisorTab : public QWidget {
    Q_OBJECT
public:
    explicit SupervisorTab(QWidget *parent = nullptr);
};
#endif // SUPERVISOR_TAB_H
