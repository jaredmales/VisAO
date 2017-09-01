# Makefile for all VisAO software

SUBSYSTEMS_TO_BUILD = lib dioserver ccd47 framewriter frameserver ccd39 Reconstructor ShutterControl frameselector FilterWheels FocusMotorCtrl GimbalMotors coronguide WollastonLift  sysmonD PwrMon VisAOIClient zaberStage HWPRotator imviewer GUIs testapps
					  
SUBSYSTEMS_TO_INSTALL = dioserver ccd47 framewriter frameserver ccd39 Reconstructor ShutterControl frameselector FilterWheels FocusMotorCtrl GimbalMotors coronguide WollastonLift PyModules PyScripts Scripts sysmonD PwrMon VisAOIClient zaberStage HWPRotator imviewer GUIs VisAOScript testapps

SUBSYSTEMS_TO_CLEAN = lib dioserver ccd47 framewriter frameserver ccd39 Reconstructor ShutterControl frameselector  FilterWheels FocusMotorCtrl GimbalMotors coronguide WollastonLift PyModules PyScripts Scripts sysmonD PwrMon VisAOIClient zaberStage HWPRotator imviewer GUIs VisAOScript testapps

SUBSYSTEMS_FOR_DEVEL = lib dioserver ShutterControl testapps

VISAO_NO_MAIN = -D__VISAO_NO_MAIN



all: adopt_parts
	for dir in ${SUBSYSTEMS_TO_BUILD}; do \
	 (cd $$dir; ${MAKE} all) || break; \
	done

devel:
	for dir in ${SUBSYSTEMS_FOR_DEVEL}; do \
	 (cd $$dir; ${MAKE} all) || break; \
	 done

adopt_parts:
	$(MAKE) -B -C $(ADOPT_SOURCE)/lib/base globals.h
	$(MAKE) -B -C $(ADOPT_SOURCE)/lib/base errordb.h
	$(MAKE) -B -C $(ADOPT_SOURCE)/lib/base msgcodes.h
	$(MAKE) -B -C $(ADOPT_SOURCE)/lib/base vartypes.h
	$(MAKE) -C $(ADOPT_SOURCE)/lib/hwlib hwlib.a
	$(MAKE) -C $(ADOPT_SOURCE)/lib/base thrdlib.a
	$(MAKE) -C $(ADOPT_SOURCE)/lib logger.a 
	$(MAKE) -C $(ADOPT_SOURCE)/lib aoapp.a
	$(MAKE) -C $(ADOPT_SOURCE)/lib configlib.a
	$(MAKE) -C $(ADOPT_SOURCE)/lib netlib.a
	$(MAKE) -C $(ADOPT_SOURCE)/lib/hwlib hwlib.a
	$(MAKE) -C $(ADOPT_SOURCE)/lib/aoslib aoscodes.h
	$(MAKE) -C $(ADOPT_SOURCE)/WFSCtrl JoeCtrl.o
	$(MAKE) -C $(ADOPT_SOURCE)/SimpleMotorCtrl SimpleMotorCtrl.o 
	$(MAKE) -C $(ADOPT_SOURCE)/SimpleMotorCtrl SimpleMotor.o 
	$(MAKE) -C $(ADOPT_SOURCE)/SimpleMotorCtrl ADCWheel.o 
	$(MAKE) -C $(ADOPT_SOURCE)/SimpleMotorCtrl Rerotator.o 
	$(MAKE) -C $(ADOPT_SOURCE)/SimpleMotorCtrl FilterWheel.o 
	$(MAKE) -C $(ADOPT_SOURCE)/SimpleMotorCtrl MCBL2805.o 
	$(MAKE) -C $(ADOPT_SOURCE)/SimpleMotorCtrl Mercury.o
	$(MAKE) -C $(ADOPT_SOURCE)/MagAO/MagAOIClient MagAOIClient.o

check_root:
	@runner=`whoami` ; \
	if test $$runner != "root" ; \
		then \
		echo "" ; \
		echo "You must be root to install VisAO." ; \
		echo "" ; \
		exit 1 ; \
	fi

install: check_root all install-paths
	for dir in ${SUBSYSTEMS_TO_INSTALL}; do \
	 (cd $$dir; ${MAKE} install)|| break; \
	done

install-paths:
	install -d $(VISAO_ROOT) --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP) --mode=755
	install -d $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP) --mode=755
	install -d $(VISAO_ROOT)/fifos --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP) --mode=755
	install -d $(VISAO_ROOT)/data --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP) --mode=755
	install -d $(VISAO_ROOT)/data/ccd47 --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP) --mode=755
	install -d $(VISAO_ROOT)/data/ccd39 --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP) --mode=755
	install -d $(VISAO_ROOT)/data/shutter --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP) --mode=755
	install -d $(VISAO_ROOT)/init --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP) --mode=755
	install -d $(VISAO_ROOT)/log --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP) --mode=755
	install -d $(VISAO_ROOT)/profile --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP) --mode=755
	install -d $(VISAO_ROOT)/lib --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP) --mode=755
	install -d $(VISAO_ROOT)/lib/python --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP) --mode=755
	install -d $(VISAO_ROOT)/lib/python/VisAO --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP) --mode=755
	install -d $(VISAO_ROOT)/lib/python/AdOpt --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP) --mode=755
	install -d $(VISAO_ROOT)/lib/python/AdOpt/WfsGUI --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP) --mode=755
	install -d $(VISAO_ROOT)/VisAOScript --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP) --mode=755

clean:
	$(MAKE) -C $(ADOPT_SOURCE)/lib clean
	$(MAKE) -C $(ADOPT_SOURCE)/WFSCtrl clean
	$(MAKE) -C $(ADOPT_SOURCE)/SimpleMotorCtrl clean
	for dir in ${SUBSYSTEMS_TO_CLEAN}; do \
	 (cd $$dir; ${MAKE} clean) || break; \
	done
	rm -f *.d *~

