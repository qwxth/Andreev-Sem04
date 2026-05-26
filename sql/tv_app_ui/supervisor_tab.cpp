#include "supervisor_tab.h"
#include "broadcast_tab.h"
#include "viewer_tab.h"
#include "view_history_tab.h"
#include <QTabWidget>
#include <QVBoxLayout>

SupervisorTab::SupervisorTab(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    QTabWidget *tabs = new QTabWidget;
    tabs->addTab(new BroadcastTab(this), "Передачи (надзор)");
    tabs->addTab(new ViewerTab(this), "Зрители (надзор)");
    tabs->addTab(new ViewHistoryTab(this), "Просмотры (надзор)");
    layout->addWidget(tabs);
    setLayout(layout);
}
