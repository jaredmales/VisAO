######################################################################
# 
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += ../ $(VISAO_LIB) $(ADOPT_LIB)

MAKEFILE = makefile.cggui

# Input
HEADERS += coronguideform.h ../basic_ui.h
FORMS += CoronGuideForm.ui
SOURCES += coronguideform.cpp CoronGuideGUI_main.cpp ../basic_ui.cpp 
LIBS += $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) build_time.o