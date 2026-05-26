/****************************************************************************
** Meta object code from reading C++ file 'broadcast_tab.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.18)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../broadcast_tab.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'broadcast_tab.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.18. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_BroadcastTab_t {
    QByteArrayData data[11];
    char stringdata0[95];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_BroadcastTab_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_BroadcastTab_t qt_meta_stringdata_BroadcastTab = {
    {
QT_MOC_LITERAL(0, 0, 12), // "BroadcastTab"
QT_MOC_LITERAL(1, 13, 7), // "refresh"
QT_MOC_LITERAL(2, 21, 0), // ""
QT_MOC_LITERAL(3, 22, 5), // "onAdd"
QT_MOC_LITERAL(4, 28, 6), // "onSave"
QT_MOC_LITERAL(5, 35, 8), // "onDelete"
QT_MOC_LITERAL(6, 44, 12), // "onSearchById"
QT_MOC_LITERAL(7, 57, 15), // "onSearchByTitle"
QT_MOC_LITERAL(8, 73, 13), // "onRowSelected"
QT_MOC_LITERAL(9, 87, 3), // "row"
QT_MOC_LITERAL(10, 91, 3) // "col"

    },
    "BroadcastTab\0refresh\0\0onAdd\0onSave\0"
    "onDelete\0onSearchById\0onSearchByTitle\0"
    "onRowSelected\0row\0col"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_BroadcastTab[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x08 /* Private */,
       3,    0,   50,    2, 0x08 /* Private */,
       4,    0,   51,    2, 0x08 /* Private */,
       5,    0,   52,    2, 0x08 /* Private */,
       6,    0,   53,    2, 0x08 /* Private */,
       7,    0,   54,    2, 0x08 /* Private */,
       8,    2,   55,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    9,   10,

       0        // eod
};

void BroadcastTab::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<BroadcastTab *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->refresh(); break;
        case 1: _t->onAdd(); break;
        case 2: _t->onSave(); break;
        case 3: _t->onDelete(); break;
        case 4: _t->onSearchById(); break;
        case 5: _t->onSearchByTitle(); break;
        case 6: _t->onRowSelected((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject BroadcastTab::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_BroadcastTab.data,
    qt_meta_data_BroadcastTab,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *BroadcastTab::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BroadcastTab::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_BroadcastTab.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int BroadcastTab::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
