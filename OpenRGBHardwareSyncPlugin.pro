QT +=                  \
    gui                \
    widgets            \
    core               \


TEMPLATE = lib
DEFINES += ORGBHARDWARESYNCPLUGIN_LIBRARY

#-----------------------------------------------------------------------------------------------#
# Application Configuration                                                                     #
#-----------------------------------------------------------------------------------------------#
PLUGIN_VERSION     = 0.1

#-----------------------------------------------------------------------------------------------#
# Automatically generated build information                                                     #
#-----------------------------------------------------------------------------------------------#
win32:BUILDDATE = $$system(date /t)
unix:BUILDDATE  = $$system(date -R -d "@${SOURCE_DATE_EPOCH:-$(date +%s)}")
GIT_COMMIT_ID   = $$system(git --git-dir $$_PRO_FILE_PWD_/.git --work-tree $$_PRO_FILE_PWD_ rev-parse HEAD)
GIT_COMMIT_DATE = $$system(git --git-dir $$_PRO_FILE_PWD_/.git --work-tree $$_PRO_FILE_PWD_ show -s --format=%ci HEAD)
GIT_BRANCH      = $$system(git --git-dir $$_PRO_FILE_PWD_/.git --work-tree $$_PRO_FILE_PWD_ rev-parse --abbrev-ref HEAD)


win32:LATEST_BUILD_URL="https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/jobs/artifacts/master/download?job=Windows 64"
unix:!macx:LATEST_BUILD_URL="https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-/jobs/artifacts/master/download?job=Linux 64 - Bullseye"

#-----------------------------------------------------------------------------------------------#
# Inject vars in defines                                                                        #
#-----------------------------------------------------------------------------------------------#
DEFINES +=                                                                                      \
    VERSION_STRING=\\"\"\"$$PLUGIN_VERSION\\"\"\"                                               \
    BUILDDATE_STRING=\\"\"\"$$BUILDDATE\\"\"\"                                                  \
    GIT_COMMIT_ID=\\"\"\"$$GIT_COMMIT_ID\\"\"\"                                                 \
    GIT_COMMIT_DATE=\\"\"\"$$GIT_COMMIT_DATE\\"\"\"                                             \
    GIT_BRANCH=\\"\"\"$$GIT_BRANCH\\"\"\"                                                       \
    LATEST_BUILD_URL=\\"\"\"$$LATEST_BUILD_URL\\"\"\"                                           \


#-------------------------------------------------------------------#
# Includes                                                          #
#-------------------------------------------------------------------#
INCLUDEPATH +=                                                      \
    OpenRGB/                                                        \
    OpenRGB/i2c_smbus                                               \
    OpenRGB/RGBController                                           \
    OpenRGB/net_port                                                \
    OpenRGB/dependencies/json                                       \

HEADERS +=                                                          \
    ControllerZone.h                                                \
    ControllerZoneListItemWidget.h                                  \
    MeasureSettings.h                                               \
    HardwareSyncMainPage.h                                          \
    MeasureColor.h                                                  \
    MeasureEntry.h                                                  \
    OpenRGB/OpenRGBPluginInterface.h                                \
    OpenRGB/ResourceManager.h                                       \
    OpenRGBHardwareSyncPlugin.h                                     \
    HardwareMeasure.h                                               \
    ColorPicker.h                                                   \
    ColorStop.h                                                     \
    PluginInfo.h                                                    \
    QTooltipedSlider.h                                              \
    TabHeader.h                                                     \
    EditableLabel.h                                                 \

SOURCES +=                                                          \
    ControllerZoneListItemWidget.cpp                                \
    MeasureSettings.cpp                                             \
    HardwareSyncMainPage.cpp                                        \
    MeasureColor.cpp                                                \
    MeasureEntry.cpp                                                \
    OpenRGBHardwareSyncPlugin.cpp                                   \
    HardwareMeasure.cpp                                             \
    ColorPicker.cpp                                                 \
    ColorStop.cpp                                                   \
    PluginInfo.cpp                                                  \
    QTooltipedSlider.cpp                                            \
    TabHeader.cpp                                                   \
    EditableLabel.cpp                                               \

FORMS +=                                                            \
    ControllerZoneListItemWidget.ui                                 \
    HardwareSyncMainPage.ui                                         \
    MeasureColor.ui                                                 \
    MeasureEntry.ui                                                 \
    ColorPicker.ui                                                  \
    ColorStop.ui                                                    \
    PluginInfo.ui                                                   \
    TabHeader.ui                                                    \

#-------------------------------------------------------------------#
# Windows GitLab CI Configuration                                   #
#-------------------------------------------------------------------#

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
    LIBS +=                                                             \
        -lws2_32                                                        \
        -lole32                                                         \
}

win32:contains(QMAKE_TARGET.arch, x86) {
    LIBS +=                                                             \
        -lws2_32                                                        \
        -lole32                                                         \
}

win32:DEFINES +=                                                        \
    _MBCS                                                               \
    WIN32                                                               \
    _CRT_SECURE_NO_WARNINGS                                             \
    _WINSOCK_DEPRECATED_NO_WARNINGS                                     \
    WIN32_LEAN_AND_MEAN                                                 \

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

#-----------------------------------------------------------------------#
# Linux-specific Configuration                                          #
#-----------------------------------------------------------------------#
unix:!macx {

    # lm sensors
    INCLUDEPATH +=                                                      \
        dependencies/libsensors-cpp                                     \

    HEADERS +=                                                          \
        dependencies/libsensors-cpp/sensors.h                           \
        dependencies/libsensors-cpp/error.h                             \

    SOURCES +=                                                          \
        dependencies/libsensors-cpp/sensors.cpp                         \
        dependencies/libsensors-cpp/error.cpp                           \

    LIBS += -lsensors
    QMAKE_CXXFLAGS += -std=c++17

    # libgtop
    INCLUDEPATH += \
        /usr/include/glib-2.0/                                          \
        /usr/include/libgtop-2.0/                                       \
        /usr/lib/x86_64-linux-gnu/glib-2.0/include/                     \

    LIBS += -lgtop-2.0 -lglib-2.0
}


RESOURCES +=                                                            \
    resources.qrc
