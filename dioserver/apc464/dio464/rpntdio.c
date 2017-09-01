//Includes modified by Jared R. Males 6/2/10

#include "../pmccommon/pmcmulticommon.h"
#include "../pmc464.h"
/*
{+D}
    SYSTEM:		Acromag Pmc464 Digital I/O Board

    MODULE NAME:	rpntdio() - read input point

    VERSION:		A

    CREATION DATE:	01/28/03

    CODED BY:		FJM

    ABSTRACT:		The module reads a input value from a single I/O point.

    CALLING SEQUENCE:	status=rpntdio(c_blk,port,point);
			  where:
			    status (long)
			      The returned value of the I/O point
			      or error flag.
			    c_blk (pointer to structure)
			      Pointer to the board memory map structure.
			    port (unsigned)
			      The target I/O port number.
			    point (unsigned)
			      The target I/O point.

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

	This module reads a value from an I/O point of an I/O port, and
	returns the current value of the point. The valid values of
	"port" are 0 to 1 and "point" are from 0 to 15; otherwise,
	a -1 is returned.  If no error, either 0 or 1 is returned.
*/

long rpntdio(c_blk, port, point)

struct pmc464 *c_blk;
unsigned port;
unsigned point; 	    /* the I/O point of a port */

{

/*
    ENTRY POINT OF ROUTINE
*/

    if (port > 3 || point > 15 )	/* error checking */
       return(-1);
    else
    {
	if ( (unsigned)input_word(c_blk->nHandle, (word*)&c_blk->brd_ptr->IOPort[port]) & (1 << point) )
	    return(1);
	else
	    return(0);
    }
}

