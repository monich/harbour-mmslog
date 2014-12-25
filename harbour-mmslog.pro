TARGET = harbour-mmslog

QT += dbus qml
CONFIG += sailfishapp
QMAKE_CXXFLAGS += -Wno-unused-parameter

CONFIG(debug, debug|release) {
  DEFINES += DEBUG
}

HEADERS += \
    src/mmsengine.h \
    src/mmsenginelog.h \
    src/mmsdebug.h \
    src/mmslogmodel.h \
    src/org.nemo.transferengine.h \
    src/sigchildaction.h \
    src/transfermethodinfo.h \
    src/transfermethodsmodel.h

SOURCES += \
    src/main.cpp \
    src/mmsengine.cpp \
    src/mmsenginelog.cpp \
    src/mmslogmodel.cpp \
    src/org.nemo.transferengine.cpp \
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
    src/org.nemo.transferengine.xml \
    translations/*.ts

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n
TRANSLATIONS += \
    translations/harbour-mmslog.ts \
    translations/harbour-mmslog-ru.ts
