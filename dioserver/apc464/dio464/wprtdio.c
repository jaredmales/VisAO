//Includes modified by Jared R. Males 6/2/10

#include "../pmccommon/pmcmulticommon.h"
#include "../pmc464.h"

/*
{+D}
    SYSTEM:		Acromag IP464 Digital I/O Board

    FILENAME:		wprt464.c

    MODULE NAME:	wprt464() - write output port

    VERSION:		A

    CREATION DATE:	01/28/03

    CODED BY:		FJM

    ABSTRACT:		This module writes output values to a single I/O port.

    CALLING SEQUENCE:	status = wprt464(c_blk, port, value);
			  where:
			    status (long)
			      The returned error status.
			    c_blk (pointer to structure)
			      Pointer to the board memory map.
			    port (unsigned)
			      The target I/O port number.
			    value (unsigned)
			      The output value.

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

    This module writes a 16-bit hex output value (from 0000 to FFFF) to
    a output port.
    The valid value of "port" is from 0 to 1. Otherwise, a -1 is returned.
*/


long wprtdio(c_blk, port, value)


struct pmc464 *c_blk;
unsigned port;
unsigned value; 	    /* the output value */

{

/*
    ENTRY POINT OF ROUTINE
*/
    if (port > 3 || value > 0xFFFF)	/* error checking */
       return(-1);

	output_word(c_blk->nHandle, (word*)&c_blk->brd_ptr->IOPort[port], value);
	return(0);
}
