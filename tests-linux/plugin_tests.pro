QT += core gui widgets
CONFIG += console c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = plugin_tests
SOURCES += plugin_tests.cpp
INCLUDEPATH += . .. ../OpenRGB ../OpenRGB/RGBController ../OpenRGB/dependencies/json
