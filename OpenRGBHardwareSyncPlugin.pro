#-----------------------------------------------------------------------------------------------#
# OpenRGB Hardware Sync Plugin QMake Project                                                    #
#-----------------------------------------------------------------------------------------------#

#-----------------------------------------------------------------------------------------------#
# Qt Configuration                                                                              #
#-----------------------------------------------------------------------------------------------#
QT +=                                                                                           \
    core                                                                                        \
    gui                                                                                         \
    widgets

DEFINES += ORGBHARDWARESYNCPLUGIN_LIBRARY
TEMPLATE = lib

#-----------------------------------------------------------------------------------------------#
# Build Configuration                                                                           #
#-----------------------------------------------------------------------------------------------#
CONFIG +=                                                                                       \
    plugin                                                                                      \
    silent                                                                                      \

#-----------------------------------------------------------------------------------------------#
# Application Configuration                                                                     #
#-----------------------------------------------------------------------------------------------#
MAJOR       = 0
MINOR       = 9
SUFFIX      = git

SHORTHASH   = $$system("git rev-parse --short=7 HEAD")
LASTTAG     = "release_"$$MAJOR"."$$MINOR
COMMAND     = "git rev-list --count "$$LASTTAG"..HEAD"
COMMITS     = $$system($$COMMAND)

VERSION_NUM = $$MAJOR"."$$MINOR"."$$COMMITS
VERSION_STR = $$MAJOR"."$$MINOR

VERSION_DEB = $$VERSION_NUM
VERSION_WIX = $$VERSION_NUM".0"
VERSION_AUR = $$VERSION_NUM
VERSION_RPM = $$VERSION_NUM

equals(SUFFIX, "git") {
VERSION_STR = $$VERSION_STR"+ ("$$SUFFIX$$COMMITS")"
VERSION_DEB = $$VERSION_DEB"~git"$$SHORTHASH
VERSION_AUR = $$VERSION_AUR".g"$$SHORTHASH
VERSION_RPM = $$VERSION_RPM"^git"$$SHORTHASH
} else {
    !isEmpty(SUFFIX) {
VERSION_STR = $$VERSION_STR"+ ("$$SUFFIX")"
VERSION_DEB = $$VERSION_DEB"~"$$SUFFIX
VERSION_AUR = $$VERSION_AUR"."$$SUFFIX
VERSION_RPM = $$VERSION_RPM"^"$$SUFFIX
    }
}

message("VERSION_NUM: "$$VERSION_NUM)
message("VERSION_STR: "$$VERSION_STR)
message("VERSION_DEB: "$$VERSION_DEB)
message("VERSION_WIX: "$$VERSION_WIX)
message("VERSION_AUR: "$$VERSION_AUR)
message("VERSION_RPM: "$$VERSION_RPM)

#-----------------------------------------------------------------------------------------------#
# Automatically generated build information                                                     #
#-----------------------------------------------------------------------------------------------#
win32:BUILDDATE = $$system(date /t)
unix:BUILDDATE  = $$system(date -R -d "@${SOURCE_DATE_EPOCH:-$(date +%s)}")
GIT_COMMIT_ID   = $$system(git --git-dir $$_PRO_FILE_PWD_/.git --work-tree $$_PRO_FILE_PWD_ rev-parse HEAD)
GIT_COMMIT_DATE = $$system(git --git-dir $$_PRO_FILE_PWD_/.git --work-tree $$_PRO_FILE_PWD_ show -s --format=%ci HEAD)
GIT_BRANCH      = $$system(git --git-dir $$_PRO_FILE_PWD_/.git --work-tree $$_PRO_FILE_PWD_ rev-parse --abbrev-ref HEAD)

#-----------------------------------------------------------------------------------------------#
# Metadata                                                                                      #
#-----------------------------------------------------------------------------------------------#
PROJECT_DESC                = "Sync colors with hardware measures"
PROJECT_NAME                = "OpenRGB Hardware Sync Plugin"
PROJECT_URL                 = "https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin"
win32:LATEST_BUILD_URL="https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/jobs/artifacts/master/download?job=Windows 64"
unix:!macx:LATEST_BUILD_URL="https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/jobs/artifacts/master/download?job=Linux 64 - Bullseye"

#-----------------------------------------------------------------------------------------------#
# Inject vars in defines                                                                        #
#-----------------------------------------------------------------------------------------------#
DEFINES +=                                                                                      \
    VERSION_STRING=\\"\"\"$$VERSION_STR\\"\"\"                                                  \
    BUILDDATE_STRING=\\"\"\"$$BUILDDATE\\"\"\"                                                  \
    GIT_COMMIT_ID=\\"\"\"$$GIT_COMMIT_ID\\"\"\"                                                 \
    GIT_COMMIT_DATE=\\"\"\"$$GIT_COMMIT_DATE\\"\"\"                                             \
    GIT_BRANCH=\\"\"\"$$GIT_BRANCH\\"\"\"                                                       \
    PROJECT_DESC=\\"\"\"$$PROJECT_DESC\\"\"\"                                                   \
    PROJECT_NAME=\\"\"\"$$PROJECT_NAME\\"\"\"                                                   \
    PROJECT_URL=\\"\"\"$$PROJECT_URL\\"\"\"                                                     \
    LATEST_BUILD_URL=\\"\"\"$$LATEST_BUILD_URL\\"\"\"                                           \

#-----------------------------------------------------------------------------------------------#
# Update version in plugin metadata json                                                        #
#-----------------------------------------------------------------------------------------------#
JSON_FILE_IN            = $$PWD/OpenRGBHardwareSyncPlugin.json.in
JSON_FILE_OUT           = $$PWD/OpenRGBHardwareSyncPlugin.json

prebuild_json.target    = prebuild_json_target
prebuild_json.depends   = FORCE
prebuild_json.commands  = $$QMAKE_STREAM_EDITOR -e \"s|VERSION_NUM|$$VERSION_NUM|g\"            \
                                                -e \"s|VERSION_STR|$$VERSION_STR|g\"            \
                                                -e \"s|GIT_COMMIT_ID|$$GIT_COMMIT_ID|g\"        \
                                                -e \"s|PROJECT_DESC|$$PROJECT_DESC|g\"          \
                                                -e \"s|PROJECT_NAME|$$PROJECT_NAME|g\"          \
                                                -e \"s|PROJECT_URL|$$PROJECT_URL|g\"            \
                                                $$JSON_FILE_IN > $$JSON_FILE_OUT                \

QMAKE_EXTRA_TARGETS    += prebuild_json
PRE_TARGETDEPS         += prebuild_json_target

#-----------------------------------------------------------------------------------------------#
# OpenRGB Plugin SDK                                                                            #
#-----------------------------------------------------------------------------------------------#
INCLUDEPATH +=                                                                                  \
    OpenRGB                                                                                     \
    OpenRGB/dependencies/json                                                                   \
    OpenRGB/RGBController                                                                       \

#-----------------------------------------------------------------------------------------------#
# Plugin Files                                                                                  #
#-----------------------------------------------------------------------------------------------#
HEADERS +=                                                                                      \
    ControllerZone.h                                                                            \
    ControllerZoneListItemWidget.h                                                              \
    ControllerZoneManager.h                                                                     \
    HardwareSyncMainPage.h                                                                      \
    MeasureColor.h                                                                              \
    MeasureEntry.h                                                                              \
    OpenRGBHardwareSyncPlugin.h                                                                 \
    HardwareMeasure.h                                                                           \
    ColorPicker.h                                                                               \
    ColorStop.h                                                                                 \
    OpenRGBPluginsFont.h                                                                        \
    PluginInfo.h                                                                                \
    QTooltipedSlider.h                                                                          \
    TabHeader.h                                                                                 \
    EditableLabel.h                                                                             \

SOURCES +=                                                                                      \
    ControllerZoneListItemWidget.cpp                                                            \
    ControllerZoneManager.cpp                                                                   \
    HardwareSyncMainPage.cpp                                                                    \
    MeasureColor.cpp                                                                            \
    MeasureEntry.cpp                                                                            \
    OpenRGBHardwareSyncPlugin.cpp                                                               \
    HardwareMeasure.cpp                                                                         \
    ColorPicker.cpp                                                                             \
    ColorStop.cpp                                                                               \
    OpenRGBPluginsFont.cpp                                                                      \
    PluginInfo.cpp                                                                              \
    QTooltipedSlider.cpp                                                                        \
    TabHeader.cpp                                                                               \
    EditableLabel.cpp                                                                           \

FORMS +=                                                                                        \
    ControllerZoneListItemWidget.ui                                                             \
    HardwareSyncMainPage.ui                                                                     \
    MeasureColor.ui                                                                             \
    MeasureEntry.ui                                                                             \
    ColorPicker.ui                                                                              \
    ColorStop.ui                                                                                \
    PluginInfo.ui                                                                               \
    TabHeader.ui                                                                                \

#-----------------------------------------------------------------------------------------------#
# Windows-specific Configuration                                                                #
#-----------------------------------------------------------------------------------------------#

win32:CONFIG += QTPLUGIN
win32:CONFIG += c++17

win32:CONFIG(debug, debug|release) {
    win32:DESTDIR = debug
}

win32:CONFIG(release, debug|release) {
    win32:DESTDIR = release
}

win32:OBJECTS_DIR = _intermediate_$$DESTDIR/.obj
win32:MOC_DIR     = _intermediate_$$DESTDIR/.moc
win32:RCC_DIR     = _intermediate_$$DESTDIR/.qrc
win32:UI_DIR      = _intermediate_$$DESTDIR/.ui

win32:contains(QMAKE_TARGET.arch, x86_64) {
    LIBS +=                                                                                     \
        -lws2_32                                                                                \
        -lole32                                                                                 \
}

win32:contains(QMAKE_TARGET.arch, x86) {
    LIBS +=                                                                                     \
        -lws2_32                                                                                \
        -lole32                                                                                 \
}

win32:DEFINES +=                                                                                \
    _MBCS                                                                                       \
    WIN32                                                                                       \
    _CRT_SECURE_NO_WARNINGS                                                                     \
    _WINSOCK_DEPRECATED_NO_WARNINGS                                                             \
    WIN32_LEAN_AND_MEAN                                                                         \

win32:{
    INCLUDEPATH += dependencies/lhwm-cpp-wrapper
    DEPENDPATH += dependencies/lhwm-cpp-wrapper
    HEADERS += dependencies/lhwm-cpp-wrapper/lhwm-cpp-wrapper.h
}

win32:CONFIG(debug, debug|release):contains(QMAKE_TARGET.arch, x86_64) {
    LIBS += -L$$PWD/dependencies/lhwm-cpp-wrapper/x64/Debug -llhwm-cpp-wrapper
}

win32:CONFIG(release, debug|release):contains(QMAKE_TARGET.arch, x86_64) {
    LIBS += -L$$PWD/dependencies/lhwm-cpp-wrapper/x64/Release -llhwm-cpp-wrapper
}

#-----------------------------------------------------------------------------------------------#
# Linux-specific Configuration                                                                  #
#-----------------------------------------------------------------------------------------------#
unix:!macx {
    # lm sensors
    INCLUDEPATH +=                                                                              \
        dependencies/libsensors-cpp                                                             \

    HEADERS +=                                                                                  \
        dependencies/libsensors-cpp/sensors.h                                                   \
        dependencies/libsensors-cpp/error.h                                                     \

    SOURCES +=                                                                                  \
        dependencies/libsensors-cpp/sensors.cpp                                                 \
        dependencies/libsensors-cpp/error.cpp                                                   \

    LIBS += -lsensors
    QMAKE_CXXFLAGS += -std=c++17 -Wno-psabi

    CONFIG += link_pkgconfig
    PKGCONFIG += glib-2.0 libgtop-2.0

    target.path=$$PREFIX/lib/openrgb/plugins/
    INSTALLS += target
}

RESOURCES +=                                                                                    \
    resources.qrc
