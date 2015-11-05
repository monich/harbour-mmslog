TARGET = harbour-mmslog

QT += dbus
CONFIG += sailfishapp
CONFIG += link_pkgconfig
PKGCONFIG += mlite5 sailfishapp
QMAKE_CXXFLAGS += -Wno-unused-parameter

CONFIG(debug, debug|release) {
  DEFINES += DEBUG
}

INCLUDEPATH += src

HEADERS += \
    src/mmsengine.h \
    src/mmsenginelog.h \
    src/mmsdebug.h \
    src/mmslogmodel.h \
    src/ofonodbustypes.h \
    src/ofonoinfosaver.h \
    src/sigchildaction.h \
    src/transfermethodinfo.h \
    src/transfermethodsmodel.h

SOURCES += \
    src/main.cpp \
    src/mmsengine.cpp \
    src/mmsenginelog.cpp \
    src/mmslogmodel.cpp \
    src/ofonoinfosaver.cpp \
    src/sigchildaction.cpp \
    src/transfermethodinfo.cpp \
    src/transfermethodsmodel.cpp

OTHER_FILES += \
    harbour-mmslog.png \
    harbour-mmslog.desktop \
    qml/main.qml \
    qml/cover/*.qml \
    qml/pages/*.qml \
    qml/cover/*.png \
    rpm/harbour-mmslog.spec \
    rpm/harbour-mmslog.changes \
    src/*.xml \
    translations/*.ts

DBUS_INTERFACES += transferengine
transferengine.files = src/org.nemo.transferengine.xml
transferengine.header_flags = -N -c OrgNemoTransferEngine -i transfermethodinfo.h
transferengine.source_flags = -N -c OrgNemoTransferEngine

DBUS_INTERFACES += ofonomanager
ofonomanager.files = src/org.ofono.Manager.xml
ofonomanager.header_flags = -N -c OrgOfonoManager -i ofonodbustypes.h
ofonomanager.source_flags = -N -c OrgOfonoManager

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n
TRANSLATIONS += \
    translations/harbour-mmslog.ts \
    translations/harbour-mmslog-ru.ts
