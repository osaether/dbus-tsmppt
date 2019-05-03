# Application version and revision
VERSION = 1.16

# Add more folders to ship with the application here
unix {
    bindir = $$(bindir)
    DESTDIR = $$(DESTDIR)
    isEmpty(bindir) {
        bindir = /usr/local/bin
    }
    INSTALLS += target diff qml
    target.path = $${DESTDIR}$${bindir}
    qml.path = $${DESTDIR}$${bindir}
    qml.files = qml/*.qml
    diff.path = $${DESTDIR}$${bindir}
    diff.files = qml/*.diff
}

LIBS += -lmodbus
QT += core network dbus
QT -= gui

TEMPLATE = app
TARGET = dbus-tsmppt
CONFIG += console
CONFIG -= app_bundle
DEFINES += VERSION=\\\"$${VERSION}\\\"

PKGCONFIG += dbus-1 libmodbus
CONFIG += link_pkgconfig

include(ext/qslog/QsLog.pri)

INCLUDEPATH += \
               ext/qslog \
               src/velib/inc \
               src

# Input
HEADERS += src/dbus_tsmppt.h \
           src/dbus_bridge.h \
           src/dbus_tsmppt_bridge.h \
           src/tsmppt.h \
           src/v_bus_node.h \
           src/velib/src/qt/v_busitem_adaptor.h \
           src/velib/src/qt/v_busitem_private_cons.h \
           src/velib/src/qt/v_busitem_private_prod.h \
           src/velib/src/qt/v_busitem_private.h \
           src/velib/src/qt/v_busitem_proxy.h \
           src/velib/inc/velib/qt/v_busitem.h \
           src/velib/inc/velib/qt/v_busitems.h

SOURCES += src/tsmppt.cpp \
           src/dbus_tsmppt.cpp \
           src/dbus_tsmppt_bridge.cpp \
           src/dbus_bridge.cpp \
           src/main.cpp \
           src/v_bus_node.cpp \
           src/velib/src/qt/v_busitem.cpp \
           src/velib/src/qt/v_busitems.cpp \
           src/velib/src/qt/v_busitem_adaptor.cpp \
           src/velib/src/qt/v_busitem_private_cons.cpp \
           src/velib/src/qt/v_busitem_private_prod.cpp \
           src/velib/src/qt/v_busitem_proxy.cpp


QMAKE_CXXFLAGS += --std=c++11


