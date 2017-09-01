# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -Iapc464 -c $<

CPPFLAGS += $(VISAO_INCLUDE)

# programs to be made
TARGETS = FocusMotorCtrl
OBJS = FocusMotorCtrl.o FocusMotorCtrl_main.o

all: $(TARGETS)
	
FocusMotorCtrl: $(OBJS)
	$(LINKPP) -o FocusMotorCtrl $(OBJS)  $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) 

install: all
	install -d $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	su -c "install FocusMotorCtrl $(VISAO_ROOT)/bin --mode=4755 --owner=root"

clean:
#	@echo $(CPP)
#	@echo $(ADOPT_INCLUDE)
	rm -f *.o *.d *~
	rm -f FocusMotorCtrl
	
