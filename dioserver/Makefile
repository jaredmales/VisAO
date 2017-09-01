# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -Iapc464 -c $<

CPPFLAGS += $(VISAO_INCLUDE) -Iapc464

# programs to be made
TARGETS = dioserver
OBJS = dioserver.o dioserver_main.o

all: 
	$(MAKE) -C apc464
	$(MAKE) dioserver
	

dioserver: $(OBJS)
	$(LINKPP) -o dioserver $(OBJS)  $(LINK_VISAO_LIB) -Lapc464 -lpmc464  $(ADOPT_PART_LIBS) 

install: all
	install -d $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	su -c "install dioserver $(VISAO_ROOT)/bin --mode=4755 --owner=root"
	install apc464/apc464_startup.sh $(VISAO_ROOT)/bin

clean:
	rm -f *.o *.d *~
	rm -f dioserver
	$(MAKE) -C apc464 clean
	
	