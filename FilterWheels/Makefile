# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -c $<


CPPFLAGS += $(VISAO_INCLUDE)


# programs to be made
TARGETS = FilterWheelLocal
OBJS = FilterWheelLocal.o FilterWheelLocal_main.o

all: $(TARGETS) 


FilterWheelLocal: $(OBJS)
	$(LINKPP) -o FilterWheelLocal $(OBJS) $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS)

install: all
	install -d $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install FilterWheelLocal $(VISAO_ROOT)/bin

clean:
#	@echo $(CPP)
#	@echo $(ADOPT_INCLUDE)
	rm -f *.o *.d *~ 
	rm -f FilterWheelLocal
