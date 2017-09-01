# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

all: btime

btime:
	$(MAKE_BUILD_TIME)
	$(QMAKE)
	$(MAKE) -f makefile.shuttergui VISAO_LIB='$(VISAO_LIB)' ADOPT_LIB='$(ADOPT_LIB)' LINK_VISAO_LIB='$(LINK_VISAO_LIB)' \
	           ADOPT_PART_LIBS='$(ADOPT_PART_LIBS)'
	
	
install: all
	install -d $(VISAO_ROOT)/bin/sims --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install ShutterGUI $(VISAO_ROOT)/bin/sims
	
clean:
	rm -f *.o *.d *~
	rm -f ShutterGUI
	rm -f moc_* qrc_* ui_*
	rm -f makefile.shuttergui
		