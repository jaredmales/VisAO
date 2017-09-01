# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

ADOPT_PYMODS_TOCOPY = cfg.py bcolors.py AOConfig.py AOConstants.py AOErrCode.py AOExcept.py AOMsgCode.py AOStates.py AOVar.py AOVarCode.py AOSCode.py daemonize.py processControl.py processList.py cfg_MAG.py

ADOPT_PYGUIS_TOCOPY = cfg_gui.py

#ADOPT_PYMODS_TOREPLACE = processList.py

VISAO_PYMODS = VisAOprocessControl.py __init__.py

install:
	for f in ${ADOPT_PYMODS_TOCOPY}; do \
	 (cp $(ADOPT_SOURCE)/PyModules/AdOpt/$$f -r $(VISAO_ROOT)/lib/python/AdOpt) || break; \
	done

	touch $(VISAO_ROOT)/lib/python/AdOpt/__init__.py

	for f in ${ADOPT_PYGUIS_TOCOPY}; do \
	 (cp $(ADOPT_SOURCE)/GUI/WfsGUI/$$f -r $(VISAO_ROOT)/lib/python/AdOpt/WfsGUI) || break; \
	done

	touch $(VISAO_ROOT)/lib/python/AdOpt/WfsGUI/__init__.py

	for f in ${VISAO_PYMODS}; do \
	 (cp $$f -r $(VISAO_ROOT)/lib/python/VisAO) || break; \
	done

clean:
	rm -f *.pyc *~

