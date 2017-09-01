# Makefile for all VisAO software test apps.

TAPPS_TO_BUILD = shutter_tester framegrabber47_sim framegrabber39_sim
					  
TAPPS_TO_INSTALL = shutter_tester framegrabber47_sim framegrabber39_sim

TAPPS_TO_CLEAN = shutter_tester framegrabber47_sim framegrabber39_sim

all: 
	for dir in ${TAPPS_TO_BUILD}; do \
	 (cd $$dir; ${MAKE} all) || break; \
	done

		
install: all 
	for dir in ${TAPPS_TO_INSTALL}; do \
	 (cd $$dir; ${MAKE} install) || break; \
	done


clean:
	for dir in ${TAPPS_TO_CLEAN}; do \
	 (cd $$dir; ${MAKE} clean) || break; \
	done
	rm -f *~

