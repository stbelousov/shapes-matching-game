QT += core gui phonon

TARGET = shapes_matching_game
TEMPLATE = app

SOURCES += main.cpp \
           widget.cpp \
           viewer.cpp \
           shape.cpp

HEADERS += widget.h \
           viewer.h \
           shape.h

FORMS   += widget.ui \
           viewer.ui
