# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -c $<

CPPFLAGS += $(VISAO_INCLUDE)

# programs to be made
TARGETS = simboard
#shmem_fgrabber_sim

all: $(TARGETS)
	
#shmem_fgrabber_sim: shmem_fgrabber_sim.o
#	$(LINKPP) -o shmem_fgrabber_sim shmem_fgrabber_sim.o $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) 

simboard: simboard.o simboard_main.o
	$(LINKPP) -o simboard simboard.o simboard_main.o $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) 

install: all
	install simboard $(VISAO_ROOT)/bin/sims --mode=4755 --owner=root

clean:
#	@echo $(CPP)
#	@echo $(ADOPT_INCLUDE)
	rm -f *.o *.d *~
	rm -f simboard
	
	