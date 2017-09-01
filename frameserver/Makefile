# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen


CPPFLAGS += $(VISAO_INCLUDE)

# programs to be made
TARGETS = frameserver
OBJS = frameserver.o frameserver_main.o


all: $(TARGETS) 
	
frameserver: $(OBJS) $(DEP)
	$(LINKPP) -o frameserver $(OBJS)  $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS)

install: all
	install -d $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	su -c "install frameserver $(VISAO_ROOT)/bin --mode=4755 --owner=root"

clean:
#	@echo $(CPP)
#	@echo $(ADOPT_INCLUDE)
	rm -f *.o *.d *~
	rm -f frameselector
	
	
