# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -Iapc464 -c $<

CPPFLAGS += $(VISAO_INCLUDE)

# programs to be made
TARGETS = HWPRotatorCtrl
OBJS = HWPRotatorCtrl.o HWPRotatorCtrl_main.o 

all: $(TARGETS)
	
	
HWPRotatorCtrl: $(OBJS)
	$(LINKPP) -o HWPRotatorCtrl $(OBJS)  $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS)

install: all
	install -d $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	su -c "install HWPRotatorCtrl $(VISAO_ROOT)/bin --mode=4755 --owner=root"
	
clean:
	rm -f *.o *.d *~
	rm -f HWPRotatorCtrl
	
