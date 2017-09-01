######################################################################
# 
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += ../ $(VISAO_LIB) $(ADOPT_LIB)

MAKEFILE = makefile.frameselectorgui

# Input
HEADERS += frameselectorform.h ../basic_ui.h
FORMS += FrameSelectorForm.ui
SOURCES += frameselectorform.cpp FrameSelectorGUI_main.cpp ../basic_ui.cpp 
LIBS += $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) build_time.o