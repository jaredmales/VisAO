######################################################################
# Automatically generated by qmake (2.01a) Mon Dec 13 23:00:47 2010
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += ../ $(VISAO_LIB) $(ADOPT_LIB)

MAKEFILE = makefile.shuttertestergui

# Input
HEADERS += shuttertesterform.h ../basic_ui.h
FORMS += ShutterTesterForm.ui
SOURCES += shuttertesterform.cpp ShutterTesterGUI_main.cpp ../basic_ui.cpp 
LIBS += $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) build_time.o