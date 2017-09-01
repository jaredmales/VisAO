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
TARGETS = VisAOIClient

all: $(TARGETS) 

VisAOIClient: VisAOIClient_main.o VisAOIClient.o VisAOIClient.h
	$(LINKPP) -o VisAOIClient VisAOIClient.o VisAOIClient_main.o $(LINK_VISAO_LIB) $(ADOPT_PART_LIBS)  -lpthread -lm -ldl \
                     $(ADOPT_SOURCE)/MagAO/MagAOIClient/MagAOIClient.o
	


install: all
	install -d $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install VisAOIClient $(VISAO_ROOT)/bin --mode=4755 --owner=$(AOSUP_USER)

clean:
	rm -f *.o *~ *.d
	rm -f VisAOIClient
