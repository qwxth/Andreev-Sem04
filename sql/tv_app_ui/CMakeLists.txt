cmake_minimum_required(VERSION 3.16)
project(tv_app_ui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Sql)

set(SOURCES
    main.cpp
    main_window.cpp
    auth_dialog.cpp
    db_manager.cpp
    utils.cpp
    admin_tab.cpp
    broadcast_tab.cpp
    viewer_tab.cpp
    view_history_tab.cpp
    reports_tab.cpp
    schedule_tab.cpp
    channel_employee_tab.cpp
    viewer_personal_tab.cpp
    supervisor_tab.cpp
)

set(HEADERS
    main_window.h
    auth_dialog.h
    db_manager.h
    utils.h
    admin_tab.h
    broadcast_tab.h
    viewer_tab.h
    view_history_tab.h
    reports_tab.h
    schedule_tab.h
    channel_employee_tab.h
    viewer_personal_tab.h
    supervisor_tab.h
)

add_executable(tv_app_ui ${SOURCES} ${HEADERS})
target_link_libraries(tv_app_ui Qt5::Core Qt5::Widgets Qt5::Sql)
