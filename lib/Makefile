# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

.c.o:
	$(CC) $(OPTIMIZE) $(CFLAGS) -DERRHAND -c $<

.cpp.o:
	$(CPP) $(OPTIMIZE) $(CPPFLAGS) -c $<

# programs to be made
TARGETS = libvisao
OBJS = libvisaoc.o libvisaocpp.o profileutils.o profiler.o fifoutils.o visaoimutils.o  VisAOApp_base.o VisAOApp_standalone.o  readcolumns.o fir_filter.o improcessing.o 

#sharedim_stack.o framegrabber.o

all: $(TARGETS) 


libvisao: $(OBJS)
	$(AR) libvisao.a $(OBJS)
	$(RANLIB) libvisao.a

clean:
#	@echo $(CPP)
#	@echo $(ADOPT_INCLUDE)
	rm -f *.o *.d *~
	rm -f libvisao.a
