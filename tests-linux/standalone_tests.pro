QT += core gui widgets
CONFIG += console c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = standalone_tests
SOURCES += standalone_tests.cpp
INCLUDEPATH += . .. ../OpenRGB ../OpenRGB/RGBController ../OpenRGB/dependencies/json
