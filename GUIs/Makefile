# Makefile for all VisAO software GUIs

GUIS_TO_BUILD = CCD47CtrlGUI FilterWheelGUI FocusMotorGUI GimbalGUI WollastonLift ShutterTesterGUI sysmonDGUI tempmonGUI bcu39GUI ShutterGUI CoronGuideGUI VisAOEngGUI
					  
GUIS_TO_INSTALL = CCD47CtrlGUI FilterWheelGUI FocusMotorGUI GimbalGUI WollastonLift  ShutterTesterGUI sysmonDGUI tempmonGUI bcu39GUI ShutterGUI CoronGuideGUI VisAOEngGUI PyGUIs

GUIS_TO_CLEAN = CCD47CtrlGUI FilterWheelGUI FocusMotorGUI GimbalGUI WollastonLift  ShutterTesterGUI sysmonDGUI tempmonGUI bcu39GUI ShutterGUI CoronGuideGUI VisAOEngGUI PyGUIs

all: 
	for dir in ${GUIS_TO_BUILD}; do \
	 (cd $$dir; ${MAKE} all) || break; \
	done

		
install: all 
	for dir in ${GUIS_TO_INSTALL}; do \
	 (cd $$dir; ${MAKE} install) || break; \
	done


clean:
	for dir in ${GUIS_TO_CLEAN}; do \
	 (cd $$dir; ${MAKE} clean) || break; \
	done
	rm -f *~

