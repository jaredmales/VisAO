# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

VISAO_GUIMODS = VisAOAppGUI.py VisAOprocess.py

install:
	for f in ${VISAO_GUIMODS}; do \
	 (cp $$f -r $(VISAO_ROOT)/lib/python/VisAO) || break; \
	done
	install sys_processes.py $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)

clean:
	rm -f *.pyc *~

