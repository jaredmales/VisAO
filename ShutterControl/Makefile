# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -Iapc464 -c $<


CPPFLAGS += $(VISAO_INCLUDE)


# programs to be made
TARGETS = ShutterControl ShutterRemoteControl
OBJS = ShutterControl.o ShutterControlDioclient.o ShutterControl_main.o 
REMOTE_OBJS = ShutterRemoteControl.o ShutterRemoteControl_main.o

all: $(TARGETS)
	

ShutterControl: $(OBJS)
	$(LINKPP) -o ShutterControl $(OBJS)  $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) 

ShutterRemoteControl: $(REMOTE_OBJS)
	$(LINKPP) -o ShutterRemoteControl $(REMOTE_OBJS)  $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS)

install: all
	install -d $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	su -c "install ShutterControl $(VISAO_ROOT)/bin --mode=4755 --owner=root"
	install ShutterRemoteControl $(VISAO_ROOT)/bin

clean:
	rm -f *.o *.d *~
	rm -f ShutterControl
	rm -f ShutterRemoteControl
	
	