DEFINES += DLANGEDITOR_LIBRARY
TARGET = DlangEditor
TEMPLATE = lib

QT += core gui

# DlangEditor files

SOURCES += dlangeditorplugin.cpp \
    dlangeditor.cpp \
    dlangindenter.cpp \
    dlangautocompleter.cpp \
    dlangcompletionassistprovider.cpp \
    dlangassistprocessor.cpp \
    dlangoptionspage.cpp \
    dcdsupport.cpp \
    dlangdebughelper.cpp

HEADERS += dlangeditorplugin.h \
    dlangeditor_global.h \
    dlangeditorconstants.h \
    dlangeditor.h \
    dlangindenter.h \
    dlangautocompleter.h \
    dlangcompletionassistprovider.h \
    dlangassistprocessor.h \
    dlangoptionspage.h \
    dcdsupport.h \
    dlangdebughelper.h

# Qt Creator linking

## set the QTC_SOURCE environment variable to override the setting here
QTCREATOR_SOURCES = $$(QTC_SOURCE)
isEmpty(QTCREATOR_SOURCES):QTCREATOR_SOURCES=/usr/src/qtcreator
!exists($$QTCREATOR_SOURCES):QTCREATOR_SOURCES=/home/cleem/myown/qtplug/qt-creator-opensource-src-3.1.1

## set the QTC_BUILD environment variable to override the setting here
IDE_BUILD_TREE = $$(QTC_BUILD)
isEmpty(IDE_BUILD_TREE):IDE_BUILD_TREE=/usr/lib/qtcreator
!exists($$IDE_BUILD_TREE):IDE_BUILD_TREE=/usr/lib/x86_64-linux-gnu/qtcreator

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

QTCREATOR_MAJOR_VERSION = $$QTCREATOR_VERSION
QTCREATOR_MAJOR_VERSION ~= s/\..*/

isEqual(QTCREATOR_MAJOR_VERSION, 2) {
  error("Only QtCreator >= 3.0.0 is supported")
}

RESOURCES += \
    dlangeditor.qrc

OTHER_FILES += \
    DlangEditor.mimetypes.xml

