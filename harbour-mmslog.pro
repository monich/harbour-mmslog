NAME = mmslog
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
    TRANSLATIONS_PATH = /usr/share/$${PREFIX}-$${NAME}/translations
}

TARGET = $${PREFIX}-$${NAME}

QT += dbus
CONFIG += sailfishapp link_pkgconfig
PKGCONFIG += mlite5 sailfishapp gio-2.0 gio-unix-2.0 glib-2.0
QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-psabi
QMAKE_CFLAGS += -Wno-unused-parameter

CONFIG(debug, debug|release) {
  DEFINES += DEBUG HARBOUR_DEBUG
}

TARGET_DATA_DIR = /usr/share/$${TARGET}

HARBOUR_LIB = src/harbour-lib
HARBOUR_LIB_INCLUDE = $${HARBOUR_LIB}/include
HARBOUR_LIB_SRC = $${HARBOUR_LIB}/src
HARBOUR_LIB_QML = $${HARBOUR_LIB}/qml

LIBGLIBUTIL = src/libglibutil
LIBGLIBUTIL_SRC = $${LIBGLIBUTIL}/src
LIBGLIBUTIL_INCLUDE = $${LIBGLIBUTIL}/include

LIBDBUSACCESS = src/libdbusaccess
LIBDBUSACCESS_SRC = $${LIBDBUSACCESS}/src
LIBDBUSACCESS_INCLUDE = $${LIBDBUSACCESS}/include

LIBDBUSLOG = src/libdbuslog
LIBDBUSLOG_COMMON = $${LIBDBUSLOG}/common
LIBDBUSLOG_COMMON_SRC = $${LIBDBUSLOG_COMMON}/src
LIBDBUSLOG_COMMON_INCLUDE = $${LIBDBUSLOG_COMMON}/include

LIBDBUSLOG_CLIENT = $${LIBDBUSLOG}/client
LIBDBUSLOG_CLIENT_SRC = $${LIBDBUSLOG_CLIENT}/src
LIBDBUSLOG_CLIENT_INCLUDE = $${LIBDBUSLOG_CLIENT}/include

SOURCES += \
    $${LIBDBUSLOG_COMMON_SRC}/dbuslog_category.c \
    $${LIBDBUSLOG_COMMON_SRC}/dbuslog_message.c

SOURCES += \
    $${LIBDBUSLOG_CLIENT_SRC}/dbuslog_client.c \
    $${LIBDBUSLOG_CLIENT_SRC}/dbuslog_receiver.c

SOURCES += \
    $${LIBDBUSACCESS_SRC}/dbusaccess_cred.c \
    $${LIBDBUSACCESS_SRC}/dbusaccess_peer.c

SOURCES += \
    $${LIBGLIBUTIL_SRC}/gutil_log.c \
    $${LIBGLIBUTIL_SRC}/gutil_misc.c \
    $${LIBGLIBUTIL_SRC}/gutil_ring.c \
    $${LIBGLIBUTIL_SRC}/gutil_strv.c

HEADERS += \
    $${HARBOUR_LIB_INCLUDE}/HarbourDebug.h \
    $${HARBOUR_LIB_INCLUDE}/HarbourSigChildHandler.h \
    $${HARBOUR_LIB_INCLUDE}/HarbourSystemInfo.h \
    $${HARBOUR_LIB_INCLUDE}/HarbourTransferMethodInfo.h \
    $${HARBOUR_LIB_INCLUDE}/HarbourTransferMethodsModel.h

SOURCES += \
    $${HARBOUR_LIB_SRC}/HarbourSigChildHandler.cpp \
    $${HARBOUR_LIB_SRC}/HarbourSystemInfo.cpp \
    $${HARBOUR_LIB_SRC}/HarbourTransferMethodInfo.cpp \
    $${HARBOUR_LIB_SRC}/HarbourTransferMethodsModel.cpp

INCLUDEPATH += \
    src \
    include \
    $${HARBOUR_LIB_INCLUDE} \
    $${LIBDBUSLOG_COMMON_INCLUDE} \
    $${LIBDBUSLOG_CLIENT_INCLUDE} \
    $${LIBDBUSACCESS_INCLUDE} \
    $${LIBGLIBUTIL_INCLUDE}

HEADERS += \
    src/appsettings.h \
    src/mmsengine.h \
    src/mmsenginelog.h \
    src/mmslogmodel.h \
    src/ofonodbustypes.h \
    src/ofonologger.h

SOURCES += \
    src/appsettings.cpp \
    src/main.cpp \
    src/mmsengine.cpp \
    src/mmsenginelog.cpp \
    src/mmslogmodel.cpp \
    src/ofonologger.cpp

OTHER_FILES += \
    harbour-$${NAME}.png \
    harbour-$${NAME}.desktop \
    qml/main.qml \
    qml/cover/*.qml \
    qml/pages/*.qml \
    qml/pages/images/*.svg \
    settings/*.qml \
    settings/harbour-$${NAME}.json \
    icons/*.svg \
    src/*.xml \
    translations/*.ts \
    privileges/* \
    rpm/*.spec

DBUS_SPEC_DIR = $$_PRO_FILE_PWD_/src/libdbuslog/spec

OTHER_FILES += \
  $${DBUS_SPEC_DIR}/org.nemomobile.Logger.xml

# org.nemomobile.Logger
DBUSLOGGER_XML = $${DBUS_SPEC_DIR}/org.nemomobile.Logger.xml
DBUSLOGGER_GENERATE = gdbus-codegen --generate-c-code \
  org.nemomobile.Logger $${DBUSLOGGER_XML}
DBUSLOGGER_H = org.nemomobile.Logger.h
org_nemomobile_Logger_h.input = DBUSLOGGER_XML
org_nemomobile_Logger_h.output = $${DBUSLOGGER_H}
org_nemomobile_Logger_h.commands = $${DBUSLOGGER_GENERATE}
org_nemomobile_Logger_h.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += org_nemomobile_Logger_h

DBUSLOGGER_C = org.nemomobile.Logger.c
org_nemomobile_Logger_c.input = DBUSLOGGER_XML
org_nemomobile_Logger_c.output = $${DBUSLOGGER_C}
org_nemomobile_Logger_c.commands = $${DBUSLOGGER_GENERATE}
org_nemomobile_Logger_c.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += org_nemomobile_Logger_c
GENERATED_SOURCES += $${DBUSLOGGER_C}

# Icons
ICON_SIZES = 86 108 128 256
for(s, ICON_SIZES) {
    icon_target = icon$${s}
    icon_dir = icons/$${s}x$${s}
    $${icon_target}.files = $${icon_dir}/$${TARGET}.png
    $${icon_target}.path = /usr/share/icons/hicolor/$${s}x$${s}/apps
    equals(PREFIX, "openrepos") {
        $${icon_target}.extra = cp $${icon_dir}/harbour-$${NAME}.png $$eval($${icon_target}.files)
        $${icon_target}.CONFIG += no_check_exist
    }
    INSTALLS += $${icon_target}
}

# harbour-lib QML components
HARBOUR_QML_COMPONENTS = \
    $${HARBOUR_LIB_QML}/HarbourHighlightIcon.qml \
    $${HARBOUR_LIB_QML}/HarbourShareMethodList.qml

qml_components.files = $${HARBOUR_QML_COMPONENTS}
qml_components.path = $${TARGET_DATA_DIR}/qml/harbour
INSTALLS += qml_components

# Settings
app_settings {
    settings_json.files = settings/$${TARGET}.json
    settings_json.path = /usr/share/jolla-settings/entries/
    equals(PREFIX, "openrepos") {
        settings_json.extra = sed s/harbour/openrepos/g settings/harbour-$${NAME}.json > $$eval(settings_json.files)
        settings_json.CONFIG += no_check_exist
    }
    INSTALLS += settings_json
}

settings_qml.files = settings/*.qml
settings_qml.path = $${TARGET_DATA_DIR}/qml/settings/
INSTALLS += settings_qml

# Priveleges
privileges.files = privileges/$${TARGET}
privileges.path = /usr/share/mapplauncherd/privileges.d/
INSTALLS += privileges

# Desktop file
equals(PREFIX, "openrepos") {
    desktop.extra = sed s/harbour/openrepos/g harbour-$${NAME}.desktop > $${TARGET}.desktop
    desktop.CONFIG += no_check_exist
}

DBUS_INTERFACES += ofonomanager
ofonomanager.files = src/org.ofono.Manager.xml
ofonomanager.header_flags = -N -c OrgOfonoManager -i ofonodbustypes.h
ofonomanager.source_flags = -N -c OrgOfonoManager

# Translations
TRANSLATION_IDBASED=-idbased
TRANSLATION_SOURCES = \
  $${_PRO_FILE_PWD_}/qml \
  $${_PRO_FILE_PWD_}/settings

TRANSLATION_FILES = \
  $${NAME} \
  $${NAME}-nl \
  $${NAME}-ru \
  $${NAME}-sv

for(t, TRANSLATION_FILES) {
    suffix = $$replace(t,-,_)
    in = $${_PRO_FILE_PWD_}/translations/harbour-$${t}
    out = $${OUT_PWD}/translations/$${PREFIX}-$${t}

    lupdate_target = lupdate_$$suffix
    $${lupdate_target}.commands = lupdate -noobsolete -locations none $${TRANSLATION_SOURCES} -ts \"$${in}.ts\" && \
        mkdir -p \"$${OUT_PWD}/translations\" &&  [ \"$${in}.ts\" != \"$${out}.ts\" ] && \
        cp -af \"$${in}.ts\" \"$${out}.ts\" || :

    qm_target = qm_$$suffix
    $${qm_target}.path = $$TRANSLATIONS_PATH
    $${qm_target}.depends = $${lupdate_target}
    $${qm_target}.commands = lrelease $$TRANSLATION_IDBASED \"$${out}.ts\" && \
        $(INSTALL_FILE) \"$${out}.qm\" $(INSTALL_ROOT)$${TRANSLATIONS_PATH}/

    QMAKE_EXTRA_TARGETS += $${lupdate_target} $${qm_target}
    INSTALLS += $${qm_target}
    OTHER_FILES += $${in}.ts
}
