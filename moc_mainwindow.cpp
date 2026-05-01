/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_LineNumberArea_t {
    QByteArrayData data[1];
    char stringdata0[15];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_LineNumberArea_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_LineNumberArea_t qt_meta_stringdata_LineNumberArea = {
    {
QT_MOC_LITERAL(0, 0, 14) // "LineNumberArea"

    },
    "LineNumberArea"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_LineNumberArea[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void LineNumberArea::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject LineNumberArea::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_LineNumberArea.data,
    qt_meta_data_LineNumberArea,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *LineNumberArea::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LineNumberArea::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LineNumberArea.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int LineNumberArea::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    return _id;
}
struct qt_meta_stringdata_PlainTextEditWithLineNumbers_t {
    QByteArrayData data[7];
    char stringdata0[99];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PlainTextEditWithLineNumbers_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PlainTextEditWithLineNumbers_t qt_meta_stringdata_PlainTextEditWithLineNumbers = {
    {
QT_MOC_LITERAL(0, 0, 28), // "PlainTextEditWithLineNumbers"
QT_MOC_LITERAL(1, 29, 25), // "updateLineNumberAreaWidth"
QT_MOC_LITERAL(2, 55, 0), // ""
QT_MOC_LITERAL(3, 56, 13), // "newBlockCount"
QT_MOC_LITERAL(4, 70, 20), // "updateLineNumberArea"
QT_MOC_LITERAL(5, 91, 4), // "rect"
QT_MOC_LITERAL(6, 96, 2) // "dy"

    },
    "PlainTextEditWithLineNumbers\0"
    "updateLineNumberAreaWidth\0\0newBlockCount\0"
    "updateLineNumberArea\0rect\0dy"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PlainTextEditWithLineNumbers[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x08 /* Private */,
       4,    2,   27,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::QRect, QMetaType::Int,    5,    6,

       0        // eod
};

void PlainTextEditWithLineNumbers::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PlainTextEditWithLineNumbers *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->updateLineNumberAreaWidth((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->updateLineNumberArea((*reinterpret_cast< const QRect(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject PlainTextEditWithLineNumbers::staticMetaObject = { {
    QMetaObject::SuperData::link<QPlainTextEdit::staticMetaObject>(),
    qt_meta_stringdata_PlainTextEditWithLineNumbers.data,
    qt_meta_data_PlainTextEditWithLineNumbers,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PlainTextEditWithLineNumbers::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlainTextEditWithLineNumbers::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PlainTextEditWithLineNumbers.stringdata0))
        return static_cast<void*>(this);
    return QPlainTextEdit::qt_metacast(_clname);
}

int PlainTextEditWithLineNumbers::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QPlainTextEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[45];
    char stringdata0[621];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 13), // "onTextChanged"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 17), // "toggleWysiwygMode"
QT_MOC_LITERAL(4, 44, 18), // "toggleMarkdownMode"
QT_MOC_LITERAL(5, 63, 8), // "openFile"
QT_MOC_LITERAL(6, 72, 8), // "saveFile"
QT_MOC_LITERAL(7, 81, 10), // "saveFileAs"
QT_MOC_LITERAL(8, 92, 7), // "newFile"
QT_MOC_LITERAL(9, 100, 10), // "insertBold"
QT_MOC_LITERAL(10, 111, 12), // "insertItalic"
QT_MOC_LITERAL(11, 124, 19), // "insertStrikeThrough"
QT_MOC_LITERAL(12, 144, 13), // "insertHeader1"
QT_MOC_LITERAL(13, 158, 13), // "insertHeader2"
QT_MOC_LITERAL(14, 172, 13), // "insertHeader3"
QT_MOC_LITERAL(15, 186, 16), // "insertBulletList"
QT_MOC_LITERAL(16, 203, 18), // "insertNumberedList"
QT_MOC_LITERAL(17, 222, 16), // "insertBlockquote"
QT_MOC_LITERAL(18, 239, 10), // "insertCode"
QT_MOC_LITERAL(19, 250, 10), // "insertLink"
QT_MOC_LITERAL(20, 261, 11), // "insertImage"
QT_MOC_LITERAL(21, 273, 20), // "insertHorizontalRule"
QT_MOC_LITERAL(22, 294, 11), // "insertTable"
QT_MOC_LITERAL(23, 306, 14), // "insertTableRow"
QT_MOC_LITERAL(24, 321, 17), // "insertTableColumn"
QT_MOC_LITERAL(25, 339, 14), // "deleteTableRow"
QT_MOC_LITERAL(26, 354, 11), // "deleteTable"
QT_MOC_LITERAL(27, 366, 17), // "deleteTableColumn"
QT_MOC_LITERAL(28, 384, 22), // "insertSpecialCharacter"
QT_MOC_LITERAL(29, 407, 15), // "insertCodeBlock"
QT_MOC_LITERAL(30, 423, 10), // "insertTask"
QT_MOC_LITERAL(31, 434, 13), // "checkSpelling"
QT_MOC_LITERAL(32, 448, 23), // "showSpellingContextMenu"
QT_MOC_LITERAL(33, 472, 8), // "position"
QT_MOC_LITERAL(34, 481, 4), // "word"
QT_MOC_LITERAL(35, 486, 21), // "showEditorContextMenu"
QT_MOC_LITERAL(36, 508, 3), // "pos"
QT_MOC_LITERAL(37, 512, 20), // "makeSelectedTextLink"
QT_MOC_LITERAL(38, 533, 8), // "showHelp"
QT_MOC_LITERAL(39, 542, 12), // "showSettings"
QT_MOC_LITERAL(40, 555, 14), // "changeLanguage"
QT_MOC_LITERAL(41, 570, 8), // "language"
QT_MOC_LITERAL(42, 579, 15), // "changeTextColor"
QT_MOC_LITERAL(43, 595, 15), // "convertEncoding"
QT_MOC_LITERAL(44, 611, 9) // "codecName"

    },
    "MainWindow\0onTextChanged\0\0toggleWysiwygMode\0"
    "toggleMarkdownMode\0openFile\0saveFile\0"
    "saveFileAs\0newFile\0insertBold\0"
    "insertItalic\0insertStrikeThrough\0"
    "insertHeader1\0insertHeader2\0insertHeader3\0"
    "insertBulletList\0insertNumberedList\0"
    "insertBlockquote\0insertCode\0insertLink\0"
    "insertImage\0insertHorizontalRule\0"
    "insertTable\0insertTableRow\0insertTableColumn\0"
    "deleteTableRow\0deleteTable\0deleteTableColumn\0"
    "insertSpecialCharacter\0insertCodeBlock\0"
    "insertTask\0checkSpelling\0"
    "showSpellingContextMenu\0position\0word\0"
    "showEditorContextMenu\0pos\0"
    "makeSelectedTextLink\0showHelp\0"
    "showSettings\0changeLanguage\0language\0"
    "changeTextColor\0convertEncoding\0"
    "codecName"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      38,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,  204,    2, 0x08 /* Private */,
       3,    0,  205,    2, 0x08 /* Private */,
       4,    0,  206,    2, 0x08 /* Private */,
       5,    0,  207,    2, 0x08 /* Private */,
       6,    0,  208,    2, 0x08 /* Private */,
       7,    0,  209,    2, 0x08 /* Private */,
       8,    0,  210,    2, 0x08 /* Private */,
       9,    0,  211,    2, 0x08 /* Private */,
      10,    0,  212,    2, 0x08 /* Private */,
      11,    0,  213,    2, 0x08 /* Private */,
      12,    0,  214,    2, 0x08 /* Private */,
      13,    0,  215,    2, 0x08 /* Private */,
      14,    0,  216,    2, 0x08 /* Private */,
      15,    0,  217,    2, 0x08 /* Private */,
      16,    0,  218,    2, 0x08 /* Private */,
      17,    0,  219,    2, 0x08 /* Private */,
      18,    0,  220,    2, 0x08 /* Private */,
      19,    0,  221,    2, 0x08 /* Private */,
      20,    0,  222,    2, 0x08 /* Private */,
      21,    0,  223,    2, 0x08 /* Private */,
      22,    0,  224,    2, 0x08 /* Private */,
      23,    0,  225,    2, 0x08 /* Private */,
      24,    0,  226,    2, 0x08 /* Private */,
      25,    0,  227,    2, 0x08 /* Private */,
      26,    0,  228,    2, 0x08 /* Private */,
      27,    0,  229,    2, 0x08 /* Private */,
      28,    0,  230,    2, 0x08 /* Private */,
      29,    0,  231,    2, 0x08 /* Private */,
      30,    0,  232,    2, 0x08 /* Private */,
      31,    0,  233,    2, 0x08 /* Private */,
      32,    2,  234,    2, 0x08 /* Private */,
      35,    1,  239,    2, 0x08 /* Private */,
      37,    0,  242,    2, 0x08 /* Private */,
      38,    0,  243,    2, 0x08 /* Private */,
      39,    0,  244,    2, 0x08 /* Private */,
      40,    1,  245,    2, 0x08 /* Private */,
      42,    0,  248,    2, 0x08 /* Private */,
      43,    1,  249,    2, 0x08 /* Private */,

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
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,   33,   34,
    QMetaType::Void, QMetaType::QPoint,   36,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   41,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   44,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onTextChanged(); break;
        case 1: _t->toggleWysiwygMode(); break;
        case 2: _t->toggleMarkdownMode(); break;
        case 3: _t->openFile(); break;
        case 4: _t->saveFile(); break;
        case 5: _t->saveFileAs(); break;
        case 6: _t->newFile(); break;
        case 7: _t->insertBold(); break;
        case 8: _t->insertItalic(); break;
        case 9: _t->insertStrikeThrough(); break;
        case 10: _t->insertHeader1(); break;
        case 11: _t->insertHeader2(); break;
        case 12: _t->insertHeader3(); break;
        case 13: _t->insertBulletList(); break;
        case 14: _t->insertNumberedList(); break;
        case 15: _t->insertBlockquote(); break;
        case 16: _t->insertCode(); break;
        case 17: _t->insertLink(); break;
        case 18: _t->insertImage(); break;
        case 19: _t->insertHorizontalRule(); break;
        case 20: _t->insertTable(); break;
        case 21: _t->insertTableRow(); break;
        case 22: _t->insertTableColumn(); break;
        case 23: _t->deleteTableRow(); break;
        case 24: _t->deleteTable(); break;
        case 25: _t->deleteTableColumn(); break;
        case 26: _t->insertSpecialCharacter(); break;
        case 27: _t->insertCodeBlock(); break;
        case 28: _t->insertTask(); break;
        case 29: _t->checkSpelling(); break;
        case 30: _t->showSpellingContextMenu((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 31: _t->showEditorContextMenu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 32: _t->makeSelectedTextLink(); break;
        case 33: _t->showHelp(); break;
        case 34: _t->showSettings(); break;
        case 35: _t->changeLanguage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 36: _t->changeTextColor(); break;
        case 37: _t->convertEncoding((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 38)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 38;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 38)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 38;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
