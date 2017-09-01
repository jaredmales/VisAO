######################################################################
# 
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += ../ $(VISAO_LIB) $(ADOPT_LIB)

MAKEFILE = makefile.shuttergui

# Input
HEADERS += shutterform.h ../basic_ui.h
FORMS += ShutterForm.ui
SOURCES += shutterform.cpp ShutterGUI_main.cpp ../basic_ui.cpp 
LIBS += $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) build_time.o