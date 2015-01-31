DEFINES += DLANGEDITOR_LIBRARY
TARGET = DlangEditor
TEMPLATE = lib

QT += core gui

# DlangEditor files

SOURCES += src/dlangeditorplugin.cpp \
    src/dlangeditor.cpp \
    src/dlangindenter.cpp \
    src/dlangautocompleter.cpp \
    src/dlangcompletionassistprovider.cpp \
    src/dlangassistprocessor.cpp \
    src/dlangoptionspage.cpp \
    src/dcdsupport.cpp \
    src/dlangdebughelper.cpp \
    src/dlanghoverhandler.cpp \
    src/dlanguseselectionupdater.cpp \
    src/locator/dlanglocatorcurrentdocumentfilter.cpp

HEADERS += src/dlangeditorplugin.h \
    src/dlangeditor_global.h \
    src/dlangeditorconstants.h \
    src/dlangeditor.h \
    src/dlangindenter.h \
    src/dlangautocompleter.h \
    src/dlangcompletionassistprovider.h \
    src/dlangassistprocessor.h \
    src/dlangoptionspage.h \
    src/dcdsupport.h \
    src/dlangdebughelper.h \
    src/dlanghoverhandler.h \
    src/dlanguseselectionupdater.h \
    src/locator/dlanglocatorcurrentdocumentfilter.h

INCLUDEPATH += src \
    thirdparty/msgpack/include/

# Qt Creator linking

## set the QTC_SOURCE environment variable to override the setting here
QTCREATOR_SOURCES = $$QTC_SOURCE
isEmpty(QTCREATOR_SOURCES):QTCREATOR_SOURCES=/usr/src/qtcreator
!exists($$QTCREATOR_SOURCES):\
    error("Set variable QTC_SOURCE to the QtCreator's sources path (current path is \"$$QTC_SOURCE\")")

## set the QTC_BUILD environment variable to override the setting here
IDE_BUILD_TREE = $$QTC_BUILD
isEmpty(IDE_BUILD_TREE):IDE_BUILD_TREE=/usr/lib/qtcreator
!exists($$IDE_BUILD_TREE): \
    error("Set variable QTC_BUILD to the QtCreator's libraries path (current path is \"$$QTC_BUILD\")")

## uncomment to build plugin into user config directory
## <localappdata>/plugins/<ideversion>
##    where <localappdata> is e.g.
##    "%LOCALAPPDATA%\QtProject\qtcreator" on Windows Vista and later
##    "$XDG_DATA_HOME/data/QtProject/qtcreator" or "~/.local/share/data/QtProject/qtcreator" on Linux
##    "~/Library/Application Support/QtProject/Qt Creator" on Mac
USE_USER_DESTDIR = yes

PROVIDER = cleem

QTC_PLUGIN_NAME = DlangEditor

LIBS += -L$$IDE_PLUGIN_PATH/QtProject \
        -L$$IDE_BUILD_TREE \
        -L$$IDE_BUILD_TREE/plugins/QtProject \
        -L$$IDE_BUILD_TREE/plugins

QTC_LIB_DEPENDS += \
    # nothing here at this time

QTC_PLUGIN_DEPENDS += \
    coreplugin \
    projectexplorer \
    cpptools

QTC_PLUGIN_RECOMMENDS += \
    # optional plugin dependencies. nothing here at this time

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)

DEFINES =

QTCREATOR_MINOR_VERSION = $$QTCREATOR_VERSION
QTCREATOR_MINOR_VERSION ~= s/\.[^\.]*$/
QTCREATOR_MINOR_VERSION ~= s/^[^\.]*\./

QTCREATOR_MAJOR_VERSION = $$QTCREATOR_VERSION
QTCREATOR_MAJOR_VERSION ~= s/\..*$/

message("Your QtCreator's sources version is $$QTCREATOR_VERSION")
message("Your QtCreator's sources major version is $$QTCREATOR_MAJOR_VERSION")
message("Your QtCreator's sources minor version is $$QTCREATOR_MINOR_VERSION")

isEqual(QTCREATOR_MAJOR_VERSION, 2) {
  error("Only QtCreator >= 3.0.0 is supported")
}

DEFINES += QTCREATOR_MAJOR_VERSION=$$QTCREATOR_MAJOR_VERSION \
    QTCREATOR_MINOR_VERSION=$$QTCREATOR_MINOR_VERSION \

RESOURCES += \
    dlangeditor.qrc

OTHER_FILES += \
    DlangEditor.mimetypes.xml

