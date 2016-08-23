openrepos {
    PREFIX = openrepos
    DEFINES += OPENREPOS
} else {
    PREFIX = harbour
}

app_settings {
    # This path is hardcoded in jolla-settings
    TRANSLATIONS_PATH = /usr/share/translations
} else {
    TRANSLATIONS_PATH = /usr/share/$${PREFIX}-mmslog/translations
}

TARGET = $${PREFIX}-mmslog
QT += dbus
CONFIG += sailfishapp link_pkgconfig
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
    settings/*.qml \
    settings/harbour-mmslog.json \
    icons/*.svg \
    src/*.xml \
    translations/*.ts \
    rpm/*.spec \
    rpm/harbour-mmslog.changes

# Icons
ICON_SIZES = 86 108 128 256
for(s, ICON_SIZES) {
    icon_target = icon$${s}
    icon_dir = icons/$${s}x$${s}
    $${icon_target}.files = $${icon_dir}/$${TARGET}.png
    $${icon_target}.path = /usr/share/icons/hicolor/$${s}x$${s}/apps
    equals(PREFIX, "openrepos") {
        $${icon_target}.extra = cp $${icon_dir}/harbour-mmslog.png $$eval($${icon_target}.files)
        $${icon_target}.CONFIG += no_check_exist
    }
    INSTALLS += $${icon_target}
}

# Settings
app_settings {
    settings_json.files = settings/$${TARGET}.json
    settings_json.path = /usr/share/jolla-settings/entries/
    equals(PREFIX, "openrepos") {
        settings_json.extra = sed s/harbour/openrepos/g settings/harbour-mmslog.json > $$eval(settings_json.files)
        settings_json.CONFIG += no_check_exist
    }
    settings_qml.files = settings/*.qml
    settings_qml.path = /usr/share/$${TARGET}/settings/
    INSTALLS += settings_qml settings_json
}

# Desktop file
equals(PREFIX, "openrepos") {
    desktop.extra = sed s/harbour/openrepos/g harbour-mmslog.desktop > $${TARGET}.desktop
    desktop.CONFIG += no_check_exist
}

# D-Bus interfaces
DBUS_INTERFACES += transferengine
transferengine.files = src/org.nemo.transferengine.xml
transferengine.header_flags = -N -c OrgNemoTransferEngine -i transfermethodinfo.h
transferengine.source_flags = -N -c OrgNemoTransferEngine

DBUS_INTERFACES += ofonomanager
ofonomanager.files = src/org.ofono.Manager.xml
ofonomanager.header_flags = -N -c OrgOfonoManager -i ofonodbustypes.h
ofonomanager.source_flags = -N -c OrgOfonoManager

# Translations
TRANSLATION_SOURCES = \
  $${_PRO_FILE_PWD_}/qml \
  $${_PRO_FILE_PWD_}/settings

TRANSLATION_FILES = mmslog mmslog-ru

for(t, TRANSLATION_FILES) {
    suffix = $$replace(t,-,_)
    in = $${_PRO_FILE_PWD_}/translations/harbour-$${t}
    out = $${OUT_PWD}/translations/$${PREFIX}-$${t}

    lupdate_target = lupdate_$$suffix
    lrelease_target = lrelease_$$suffix

    $${lupdate_target}.commands = lupdate -noobsolete $${TRANSLATION_SOURCES} -ts \"$${in}.ts\" && \
        mkdir -p \"$${OUT_PWD}/translations\" &&  [ \"$${in}.ts\" != \"$${out}.ts\" ] && \
        cp -af \"$${in}.ts\" \"$${out}.ts\" || :

    $${lrelease_target}.target = \"$${out}.qm\"
    $${lrelease_target}.depends = $${lupdate_target}
    $${lrelease_target}.commands = lrelease -idbased \"$${out}.ts\"

    QMAKE_EXTRA_TARGETS += $${lrelease_target} $${lupdate_target}
    PRE_TARGETDEPS += \"$${out}.qm\"
    qm.files += \"$${out}.qm\"
}

qm.path = $$TRANSLATIONS_PATH
qm.CONFIG += no_check_exist
INSTALLS += qm
