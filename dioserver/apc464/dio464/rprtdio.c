//Includes modified by Jared R. Males 6/2/10

#include "../pmccommon/pmcmulticommon.h"
#include "../pmc464.h"

/*
{+D}
    SYSTEM:		Acromag Pmc464 Digital I/O Board

    MODULE NAME:	rprtdio() - read input port

    VERSION:		A

    CREATION DATE:	01/28/03

    CODED BY:		FJM

    ABSTRACT:		The module reads an input value from a single I/O port.

    CALLING SEQUENCE:	status = rprtdio(c_blk, port);
			  where:
			    status (long)
			      The returned value of the I/O port
			      or error flag.
			    c_blk (pointer to structure)
			      Pointer to the board memory map.
			    port (unsigned)
			      The target I/O port number.

    MODULE TYPE:	long

    I/O RESOURCES:

    SYSTEM RESOURCES:

    MODULES CALLED:

    REVISIONS:

      DATE	BY	PURPOSE
    --------   -----	---------------------------------------------------

{-D}
*/


/*
    MODULES FUNCTION DETAILS

	This module reads a value from an I/O port, and returns the
	current value of all points in that port. The valid value
	of "port" is from 0 to 1; otherwise, a -1 is returned.
*/


long rprtdio(c_blk, port)

struct pmc464 *c_blk;
unsigned port;

{

/*
    ENTRY POINT OF THE ROUTINE
*/
    if (port > 3 )		/* error checking */
	    return(-1);
    else
	    return ((long)input_word(c_blk->nHandle, (word*)&c_blk->brd_ptr->IOPort[port]));
}
