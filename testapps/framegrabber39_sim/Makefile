# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -c $<

CPPFLAGS += $(VISAO_INCLUDE) -O3

# programs to be made
TARGETS = framegrabber39_sim 
#shmem_fgrabber_sim

all: $(TARGETS)
	
#shmem_fgrabber_sim: shmem_fgrabber_sim.o
#	$(LINKPP) -o shmem_fgrabber_sim shmem_fgrabber_sim.o $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) 

framegrabber39_sim: framegrabber39_sim.o framegrabber39_sim_main.o
	$(LINKPP) -o framegrabber39_sim framegrabber39_sim.o framegrabber39_sim_main.o $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) 

install: all
	install -d $(VISAO_ROOT)/bin/sims --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install -d $(VISAO_ROOT)/bin/sims/data/ --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install -d $(VISAO_ROOT)/bin/sims/data/sim39 --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install framegrabber39_sim $(VISAO_ROOT)/bin/sims --mode=4755 --owner=root
#install data/sim39/* $(VISAO_ROOT)/bin/sims/data/sim39 --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)

clean:
#	@echo $(CPP)
#	@echo $(ADOPT_INCLUDE)
	rm -f *.o *.d *~
	rm -f framegrabber39_sim
	
	