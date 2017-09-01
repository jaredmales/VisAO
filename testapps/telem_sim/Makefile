# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -c $<

CPPFLAGS += $(VISAO_INCLUDE)

# programs to be made
TARGETS = telem_sim 
#shmem_fgrabber_sim

all: $(TARGETS)
	
#shmem_fgrabber_sim: shmem_fgrabber_sim.o
#	$(LINKPP) -o shmem_fgrabber_sim shmem_fgrabber_sim.o $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) 

telem_sim: telem_sim_main.o
	$(LINKPP) -o telem_sim telem_sim_main.o $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) 

install: all
	install telem_sim $(VISAO_ROOT)/bin/sims --mode=4755 --owner=root

clean:
#	@echo $(CPP)
#	@echo $(ADOPT_INCLUDE)
	rm -f *.o *.d *~
	rm -f telem_sim
	
	