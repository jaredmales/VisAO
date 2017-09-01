# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

#.c.o:
#	$(CC) $(OPTIMIZE) $(CFLAGS) $(VISAO_INCLUDE) -I$(EDTDIR) -c $<
#	
#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) $(VISAO_INCLUDE) -I$(EDTDIR) -c $<

CFLAGS += $(VISAO_INCLUDE)  -I$(EDTDIR)
CPPFLAGS += $(VISAO_INCLUDE)  -I$(EDTDIR)

# programs to be made
TARGETS = loadpatterns

all: $(TARGETS) 


loadpatterns: loadpatterns.o
	$(LINKPP) -o loadpatterns loadpatterns.o $(ADOPT_PART_LIBS)  $(LINK_VISAO_LIB)  -L$(EDTDIR) -lpdv -lpthread -lm -ldl $(ADOPT_PART_LIBS)
	

clean:
#	@echo $(CPP)
#	@echo $(ADOPT_INCLUDE)
	rm -f *.o *~ *.d
	rm -f loadpatterns
