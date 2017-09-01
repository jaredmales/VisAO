#makefile for pmc464.a
#modified from the demo code provided by acromag
#by Jared R. Males, 6/2/10
#this creates a static library.


libpmc464.a:	pmcmulticommon.o\
		cnfgdio.o  rpntdio.o rprtdio.o rstsdio.o wpntdio.o wprtdio.o apc464_dioserver.o

		ar crv libpmc464.a pmcmulticommon.o\
		cnfgdio.o rpntdio.o rprtdio.o rstsdio.o wpntdio.o wprtdio.o\
		apc464_dioserver.o



#
#	Use PMC Multi common
#
pmcmulticommon.o:	pmccommon/pmcmulticommon.c pmccommon/pmcmulticommon.h
			gcc -c pmccommon/pmcmulticommon.c

#
#	Bring In Digital I/O Demonstration
#
cnfgdio.o:		dio464/cnfgdio.c pmc464.h
			gcc  -c dio464/cnfgdio.c

rpntdio.o:		dio464/rpntdio.c pmc464.h
			gcc -c dio464/rpntdio.c

rprtdio.o:  	dio464/rprtdio.c pmc464.h
			gcc  -c dio464/rprtdio.c

rstsdio.o:  	dio464/rstsdio.c pmc464.h
			gcc  -c dio464/rstsdio.c

wpntdio.o:		dio464/wpntdio.c pmc464.h
			gcc  -c dio464/wpntdio.c

wprtdio.o:		dio464/wprtdio.c pmc464.h
			gcc  -c dio464/wprtdio.c

apc464_dioserver.o:	apc464_dioserver.c apc464_dioserver.h
			gcc  -c apc464_dioserver.c


clean:
	rm -f *.o
	rm -f *~
	rm -f *.a
	


