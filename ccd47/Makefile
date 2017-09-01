# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.c.o:
#	$(CC) $(OPTIMIZE) $(CFLAGS) $(VISAO_INCLUDE) -I$(EDTDIR) -c $<
#	
#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -I$(EDTDIR) -c $<

CFLAGS += $(VISAO_INCLUDE)  -I$(EDTDIR)
CPPFLAGS += $(VISAO_INCLUDE)  -I$(EDTDIR)

# programs to be made
TARGETS = framegrabber47_main CCD47Ctrl VisAOJoeCtrl

all: $(TARGETS) 

# Dependencies
#include $(ADOPT_LIB)/makefile.aoapp
#include $(ADOPT_LIB)/hwlib/makefile.hwlib

#VisAOJoeCtrl_main: VisAOJoeCtrl_main.o VisAOJoeCtrl.o
#	$(LINKPP) -o VisAOJoeCtrl VisAOJoeCtrl.o VisAOJoeCtrl_main.o $(ADOPT_SOURCE)/WFSCtrl/JoeCtrl.o $(ADOPT_PART_LIBS) $(LINK_VISAO_LIB)

framegrabber47_main: framegrabber47_main.o framegrabber47.o
	$(LINKPP) -o framegrabber47 framegrabber47.o framegrabber47_main.o $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS)  -L$(EDTDIR) -lpdv -lpthread -lm -ldl
	
CCD47Ctrl: CCD47Ctrl.o CCD47Ctrl_main.o EDTutils.o
	$(LINKPP) -o CCD47Ctrl CCD47Ctrl.o CCD47Ctrl_main.o EDTutils.o $(ADOPT_PART_LIBS) $(LINK_VISAO_LIB) -L$(EDTDIR) -lpdv -lpthread -lm -ldl

VisAOJoeCtrl: VisAOJoeCtrl.o VisAOJoeCtrl_main.o 
	$(LINKPP) -o VisAOJoeCtrl VisAOJoeCtrl.o VisAOJoeCtrl_main.o $(ADOPT_PART_LIBS) $(LINK_VISAO_LIB)  -lpthread -lm -ldl

install: all
	install -d $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	#install VisAOJoeCtrl $(VISAO_ROOT)/bin
	su -c "install framegrabber47 $(VISAO_ROOT)/bin --mode=4755 --owner=root"
	su -c "install CCD47Ctrl $(VISAO_ROOT)/bin --mode=4755 --owner=root"
	install VisAOJoeCtrl $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
clean:
#	@echo $(CPP)
#	@echo $(ADOPT_INCLUDE)
	rm -f *.o *~ *.d
	rm -f VisAOJoeCtrl
	rm -f framegrabber47
	rm -f CCD47Ctrl
