# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -c $<

CPPFLAGS += $(VISAO_INCLUDE)

# programs to be made
TARGETS = framewriter
OBJS = framewriter_main.o

TARGETSUC = framewriterUC
OBJSUC =  framewriterUC_main.o

DEP = framewriter.h

all: $(TARGETS) $(TARGETSUC)
	
framewriter: $(OBJS) $(DEP)
	$(LINKPP) -o framewriter $(OBJS)  $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) 

framewriterUC: $(OBJSUC) $(DEP)
	$(LINKPP) -o framewriterUC $(OBJSUC)  $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS)


install: all
	install -d $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	su -c  "install framewriter $(VISAO_ROOT)/bin --mode=4755 --owner=root"
	su -c  "install framewriterUC $(VISAO_ROOT)/bin --mode=4755 --owner=root"

clean:
#	@echo $(CPP)
#	@echo $(ADOPT_INCLUDE)
	rm -f *.o *.d *~
	rm -f framewriter
	rm -f framewriterUC
	
	
