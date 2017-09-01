# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -c $<

%.o: %.cu
	nvcc -c $<

GPUINC = -I/usr/local/cuda/include/

CPPFLAGS += $(VISAO_INCLUDE) -D$(VISAO_REC_GPU) -I/home/jaredmales/include/ #--std=c++0x
CFLAGS += $(VISAO_INCLUDE) -D$(VISAO_REC_GPU) 

# programs to be made
TARGETS = reconstructor


OBJS =  svbksb.o recmat.o

#make sure gslcblas comes after atlas and clbas, so it is only used if necessary
LIBS = -lgsl -L$(ATLAS_LIB_PATH) -latlas -lcblas -latlas -lgslcblas

ifeq ($(VISAO_REC_GPU), REC_USE_GPU)
	CPPFLAGS += $(GPUINC)
	CFLAGS += $(GPUINC)
	OBJS += gpurecon.o
	LIBS += -L/usr/local/cuda/lib64/ -lcuda -lcublas
endif

all: $(TARGETS)
	
reconstructor: $(OBJS) reconstructor.o reconstructor_main.o
	$(LINKPP) -o reconstructor  reconstructor.o reconstructor_main.o $(OBJS)  $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS) $(LIBS)

Reconstruct: $(OBJS) Reconstruct.o
	$(LINKPP) -o Reconstruct Reconstruct.o $(OBJS) $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS)  $(GPULIB) $(LIBS) -L/home/jaredmales/lib/ -lmxlib -lboost_filesystem -lboost_system

#-lgsl -lgslcblas

install: all
	install -d $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install -d $(VISAO_ROOT)/bin/sims/data --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install -d $(VISAO_ROOT)/calib/visao/reconstructor/filters/ --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	su -c  "install reconstructor $(VISAO_ROOT)/bin --mode=4755 --owner=root"
#su -c  "install Reconstruct $(VISAO_ROOT)/bin --mode=4755 --owner=root"
	install data/caos_matint412_matint.txt $(VISAO_ROOT)/bin/sims/data --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install data/caos_mirdef412_vars.txt $(VISAO_ROOT)/bin/sims/data --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install filters/*.txt $(VISAO_ROOT)/calib/visao/reconstructor/filters/ --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
clean:
#	@echo $(CPP)
#	@echo $(ADOPT_INCLUDE)
	rm -f *.o *.d *~
	rm -f reconstructor
	rm -f Reconstruct
	
	
