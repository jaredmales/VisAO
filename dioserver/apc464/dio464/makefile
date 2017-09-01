
MODCFLAGS :=

a.out:		pmcmulticommon.o cnfgdio.o drvrdio.o rpntdio.o\
			rprtdio.o rstsdio.o wpntdio.o wprtdio.o
			cc -o a.out pmcmulticommon.o cnfgdio.o drvrdio.o\
			rpntdio.o rprtdio.o rstsdio.o wpntdio.o wprtdio.o

pmcmulticommon.o:	./../../pmccommon/pmcmulticommon.c ./../../pmccommon/pmcmulticommon.h
			$(CC) $(MODCFLAGS) -c ./../../pmccommon/pmcmulticommon.c

drvrdio.o:		drvrdio.c ./../pmc464.h
			$(CC) $(MODCFLAGS) -c drvrdio.c

cnfgdio.o:		cnfgdio.c ../pmc464.h
			$(CC) $(MODCFLAGS) -c cnfgdio.c

rpntdio.o:		rpntdio.c ../pmc464.h
			$(CC) $(MODCFLAGS) -c rpntdio.c

rprtdio.o:		rprtdio.c ../pmc464.h
			$(CC) $(MODCFLAGS) -c rprtdio.c

rstsdio.o:		rstsdio.c ../pmc464.h
			$(CC) $(MODCFLAGS) -c rstsdio.c

wpntdio.o:		wpntdio.c ../pmc464.h
			$(CC) $(MODCFLAGS) -c wpntdio.c

wprtdio.o:		wprtdio.c ../pmc464.h
			$(CC) $(MODCFLAGS) -c wprtdio.c
