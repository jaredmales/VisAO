# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -c $<


CPPFLAGS += $(VISAO_INCLUDE)


# programs to be made
TARGETS = PwrMon
OBJS = PwrMon.o PwrMon_main.o

all: $(TARGETS) 


PwrMon: $(OBJS) 
	$(LINKPP) -o PwrMon $(OBJS) $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS)

install: all
	install -d $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install PwrMon $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)

clean:
	rm -f *.o *.d *~ 
	rm -f PwrMon
