# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -Iapc464 -c $<

CPPFLAGS += $(VISAO_INCLUDE)

# programs to be made
TARGETS = zaberStageCtrl
OBJS = zaberStageCtrl.o zaberStageCtrl_main.o zaberStage.o zb_serial.o

all: $(TARGETS)
	
zb_serial.o: zaber/zb_serial.o zaber/zb_serial.h zaber/zb_serial.c zaber/z_common.h
	
zaberStageCtrl: $(OBJS)
	$(LINKPP) -o zaberStageCtrl $(OBJS)  $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS)

install: all
	install -d $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	su -c "install zaberStageCtrl $(VISAO_ROOT)/bin --mode=4755 --owner=root"

clean:
	rm -f *.o *.d *~
	rm -f zaberStageCtrl
	
