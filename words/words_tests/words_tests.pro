TARGET = words_tests
TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle

ROOT_DIR = ../..
DEPENDENCIES = words coding base zlib bzip2

include($$ROOT_DIR/common.pri)

SOURCES += $$ROOT_DIR/testing/testingmain.cpp \
  sorted_index_test.cpp \

HEADERS += \
  dictionary_mock.hpp \
