# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -Iapc464 -c $<

CPPFLAGS += $(VISAO_INCLUDE) -ggdb


# programs to be made
TARGETS = sysmonD
OBJS = QTITempProbe.o sysmonD.o sysmonD_main.o

all: $(TARGETS)
	
sysmonD: $(OBJS)
	$(LINKPP) -o sysmonD $(OBJS)  $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) 

install: all
	install -d $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	su -c "install sysmonD $(VISAO_ROOT)/bin --mode=4755 --owner=root"

clean:
#	@echo $(CPP)
#	@echo $(ADOPT_INCLUDE)
	rm -f *.o *.d *~
	rm -f sysmonD
	
