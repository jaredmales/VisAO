# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -c $<

CPPFLAGS += $(VISAO_INCLUDE) -ggdb

# programs to be made
TARGETS = framegrabber47_sim 
#shmem_fgrabber_sim

all: $(TARGETS)
	
#shmem_fgrabber_sim: shmem_fgrabber_sim.o
#	$(LINKPP) -o shmem_fgrabber_sim shmem_fgrabber_sim.o $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) 

framegrabber47_sim: framegrabber47_sim.o framegrabber47_sim_main.o
	$(LINKPP) -o framegrabber47_sim framegrabber47_sim.o framegrabber47_sim_main.o $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) 

install: all
	install -d $(VISAO_ROOT)/bin/sims --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install -d $(VISAO_ROOT)/bin/sims/data/1024x1024 --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install -d $(VISAO_ROOT)/bin/sims/data/1024x1024_tt_ip_3.53 --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install -d $(VISAO_ROOT)/bin/sims/data/1024x1024_tt_ip_unsat --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install -d $(VISAO_ROOT)/bin/sims/data/1024x1024_tt_ip_unsat/darks --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install -d $(VISAO_ROOT)/bin/sims/data/128x128 --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install framegrabber47_sim $(VISAO_ROOT)/bin/sims --mode=4755 --owner=root
	install data/1024x1024/* $(VISAO_ROOT)/bin/sims/data/1024x1024 --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
#install data/1024x1024_tt_ip_3.53/*.fits $(VISAO_ROOT)/bin/sims/data/1024x1024_tt_ip_3.53 --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
#	install data/1024x1024_tt_ip_unsat/*.fits $(VISAO_ROOT)/bin/sims/data/1024x1024_tt_ip_unsat --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
#	install data/1024x1024_tt_ip_unsat/fits.list $(VISAO_ROOT)/bin/sims/data/1024x1024_tt_ip_unsat --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
#	install data/1024x1024_tt_ip_unsat/darks/*.fits $(VISAO_ROOT)/bin/sims/data/1024x1024_tt_ip_unsat/darks --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
#	install data/1024x1024_tt_ip_unsat/darks/fits.list $(VISAO_ROOT)/bin/sims/data/1024x1024_tt_ip_unsat/darks --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)

	install data/128x128/* $(VISAO_ROOT)/bin/sims/data/128x128 --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)

clean:
#	@echo $(CPP)
#	@echo $(ADOPT_INCLUDE)
	rm -f *.o *.d *~
	rm -f framegrabber47_sim
	
	