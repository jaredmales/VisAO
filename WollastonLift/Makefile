# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -Iapc464 -c $<


CPPFLAGS += $(VISAO_INCLUDE)

# programs to be made
TARGETS = WollastonStatus
OBJS = WollastonStatus.o WollastonStatus_main.o

all: $(TARGETS)
	
WollastonStatus: $(OBJS)
	$(LINKPP) -o WollastonStatus $(OBJS)  $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) 

install: all
	install -d $(VISAO_ROOT)/bin
	install WollastonStatus $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)

clean:
	rm -f *.o *.d *~
	rm -f WollastonStatus
	
