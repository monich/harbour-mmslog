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
    qml/main.qml \
    qml/cover/*.qml \
    qml/pages/*.qml \
    qml/cover/*.png \
    src/org.nemo.transferengine.xml \
    rpm/harbour-mmslog.spec \
    rpm/harbour-mmslog.changes \
    harbour-mmslog.png \
    harbour-mmslog.desktop
