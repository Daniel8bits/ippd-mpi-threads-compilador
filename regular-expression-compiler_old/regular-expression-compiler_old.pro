TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        common.cpp \
        lexicon_tokenizer.cpp \
        main.cpp \
        regular_expression.cpp \
        utils.cpp

HEADERS += \
    common.h \
    lexicon_tokenizer.h \
    regular_expression.h \
    utils.h

LIBS += -pthread

