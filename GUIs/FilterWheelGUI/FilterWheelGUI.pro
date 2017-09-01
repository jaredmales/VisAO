

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH +=  ../ $(VISAO_LIB) $(ADOPT_LIB)

MAKEFILE = makefile.filterwheelgui

# Input
HEADERS += basicfilterwheelform.h ../basic_ui.h
FORMS += BasicFilterWheelForm.ui
SOURCES += basicfilterwheelform.cpp FilterWheelGUI_main.cpp ../basic_ui.cpp
LIBS += $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) build_time.o
RESOURCES += ../visao.qrc