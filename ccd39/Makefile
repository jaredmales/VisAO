# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

KERNEL_INCLUDES?=include/

CFLAGS += $(VISAO_INCLUDE)  -I$(KERNEL_INCLUDES) 
CPPFLAGS += $(VISAO_INCLUDE)  -I$(KERNEL_INCLUDES) 

# programs to be made
TARGETS = framegrabber39_main 

all: $(TARGETS) 


framegrabber39_main: framegrabber39_main.o framegrabber39.o getethertype.o
	$(LINKPP) -o framegrabber39 framegrabber39.o framegrabber39_main.o getethertype.o $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) -lc
	

install: all
	install -d $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	#install VisAOJoeCtrl $(VISAO_ROOT)/bin
	su -c "install framegrabber39 $(VISAO_ROOT)/bin --mode=4755 --owner=root"

clean:
#	@echo $(CPP)
#	@echo $(ADOPT_INCLUDE)
	rm -f *.o *~ *.d
	rm -f framegrabber39
