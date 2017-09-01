# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

all: btime

btime:
	$(MAKE_BUILD_TIME)
	$(QMAKE)
	$(MAKE) -f makefile.gimbal VISAO_LIB='$(VISAO_LIB)' ADOPT_LIB='$(ADOPT_LIB)' LINK_VISAO_LIB='$(LINK_VISAO_LIB)' \
	           ADOPT_PART_LIBS='$(ADOPT_PART_LIBS)'

install: all
	install -d $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install GimbalGUI $(VISAO_ROOT)/bin/
	
clean:
	rm -f *.o *.d *~
	rm -f GimbalGUI
	rm -f moc_* qrc_* ui_*
	rm -f makefile.gimbal
		