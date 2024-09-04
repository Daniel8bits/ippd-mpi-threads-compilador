TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    ControllerImpl.cpp \
    DistributorImpl.cpp \
    GathererImpl.cpp \
    PodImpl.cpp \
    SchedulerImpl.cpp

HEADERS += \
    ControllerImpl.h \
    DistributorImpl.h \
    GathererImpl.h \
    PodImpl.h \
    SchedulerImpl.h
