system=VISAO

ifeq ($(system),VISAO)
#This assumes that the VisAO development environment is set up
#i.e. by running install_environment.sh

# General VisAO makefile
include $(VISAO_SOURCE)/makefile.gen

PRO_FILE=imviewer.pro 
endif

ifeq ($(system),SCEXAO)
MAKE_BUILD_TIME=echo
QMAKE=/usr/share/qt4/bin/qmake
PRO_FILE=imviewer_scexao.pro
QWT_INCLUDE=/usr/local/qwt-5.2.1/include/
export QWT_INCLUDE
QWT_LIB=-L/usr/local/qwt-5.2.1/lib/ -lqwt
export QWT_LIB
F2C_LIB=-lf2c
export F2C_LIB
endif


all: levmar imviewer

imviewer:
	$(MAKE_BUILD_TIME)
	$(QMAKE) -makefile $(PRO_FILE)
	$(MAKE) -f makefile.imviewer


levmar:
	cd levmar-2.5; ${MAKE} all

ifeq ($(system),VISAO)
install: all
	install -d $(VISAO_ROOT)/bin --owner=$(AOSUP_USER) --group=$(AOSUP_GROUP)
	install imviewer $(VISAO_ROOT)/bin
endif

clean:
	rm -f *.o *.d *~
	rm -f imviewer
	rm -f imviewer_scexao
	rm -f moc_* qrc_* ui_*
	rm -f makefile.imviewer
	cd levmar-2.5; ${MAKE} clean

