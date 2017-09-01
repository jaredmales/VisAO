# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -c $<

CPPFLAGS += $(VISAO_INCLUDE)

# programs to be made
TARGETS = coronguide 
OBJS = coronguide.o coronguide_main.o


DEP = coronguide.h

all: $(TARGETS) $(TARGETSUC)
	
coronguide: $(OBJS) $(DEP)
	$(LINKPP) -o coronguide $(OBJS)  $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) 



install: all
	install -d $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	su -c  "install coronguide $(VISAO_ROOT)/bin --mode=4755 --owner=root"

clean:
	rm -f *.o *.d *~
	rm -f coronguide
	
	
	
