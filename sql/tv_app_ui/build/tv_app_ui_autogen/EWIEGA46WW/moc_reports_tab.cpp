/****************************************************************************
** Meta object code from reading C++ file 'reports_tab.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.18)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../reports_tab.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'reports_tab.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.18. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ReportsTab_t {
    QByteArrayData data[11];
    char stringdata0[213];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ReportsTab_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ReportsTab_t qt_meta_stringdata_ReportsTab = {
    {
QT_MOC_LITERAL(0, 0, 10), // "ReportsTab"
QT_MOC_LITERAL(1, 11, 20), // "reportChannelsByCity"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 27), // "reportChannelWithBroadcasts"
QT_MOC_LITERAL(4, 61, 21), // "reportViewersByStatus"
QT_MOC_LITERAL(5, 83, 19), // "reportViewerHistory"
QT_MOC_LITERAL(6, 103, 22), // "reportBroadcastViewers"
QT_MOC_LITERAL(7, 126, 30), // "reportChannelBroadcastsViewers"
QT_MOC_LITERAL(8, 157, 20), // "reportChannelViewers"
QT_MOC_LITERAL(9, 178, 18), // "reportMyBroadcasts"
QT_MOC_LITERAL(10, 197, 15) // "reportMyAccount"

    },
    "ReportsTab\0reportChannelsByCity\0\0"
    "reportChannelWithBroadcasts\0"
    "reportViewersByStatus\0reportViewerHistory\0"
    "reportBroadcastViewers\0"
    "reportChannelBroadcastsViewers\0"
    "reportChannelViewers\0reportMyBroadcasts\0"
    "reportMyAccount"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ReportsTab[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x08 /* Private */,
       3,    0,   60,    2, 0x08 /* Private */,
       4,    0,   61,    2, 0x08 /* Private */,
       5,    0,   62,    2, 0x08 /* Private */,
       6,    0,   63,    2, 0x08 /* Private */,
       7,    0,   64,    2, 0x08 /* Private */,
       8,    0,   65,    2, 0x08 /* Private */,
       9,    0,   66,    2, 0x08 /* Private */,
      10,    0,   67,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ReportsTab::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ReportsTab *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->reportChannelsByCity(); break;
        case 1: _t->reportChannelWithBroadcasts(); break;
        case 2: _t->reportViewersByStatus(); break;
        case 3: _t->reportViewerHistory(); break;
        case 4: _t->reportBroadcastViewers(); break;
        case 5: _t->reportChannelBroadcastsViewers(); break;
        case 6: _t->reportChannelViewers(); break;
        case 7: _t->reportMyBroadcasts(); break;
        case 8: _t->reportMyAccount(); break;
        default: ;
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject ReportsTab::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ReportsTab.data,
    qt_meta_data_ReportsTab,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ReportsTab::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ReportsTab::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ReportsTab.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ReportsTab::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
