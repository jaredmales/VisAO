# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -Iapc464 -c $<

CPPFLAGS += $(VISAO_INCLUDE)

# programs to be made
TARGETS = ShutterTester
OBJS = shutter_tester.o shutter_tester_main.o

all: $(TARGETS)
	
ShutterTester: $(OBJS)
	$(LINKPP) -o ShutterTester $(OBJS)  $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) 

install: all
	install -d $(VISAO_ROOT)/bin/sims --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	su -c "install ShutterTester $(VISAO_ROOT)/bin/sims --mode=4755 --owner=root"
	install -d $(VISAO_ROOT)/bin/sims/data/ --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install data/* $(VISAO_ROOT)/bin/sims/data/ --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)

clean:
#	@echo $(CPP)
#	@echo $(ADOPT_INCLUDE)
	rm -f *.o *.d *~
	rm -f ShutterTester

	
	