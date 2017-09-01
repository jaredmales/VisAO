//Includes modified by Jared R. Males 6/2/10

#include "../pmccommon/pmcmulticommon.h"
#include "../pmc464.h"

/*
{+D}
    SYSTEM:		Acromag Pmc408 Digital I/O  Board

    FILENAME:		wpntdio.c

    MODULE
	NAME:		wpntdio() - write output point

    VERSION:		A

    CREATION DATE:	01/28/03

    CODED BY:		FJM

    ABSTRACT:		Module writes an output value to a single I/O point.

    CALLING SEQUENCE:	status = wpntdio(c_blk, port, point, value);
			  where:
			    status (long)
			      The returned error status.
			    c_blk (pointer to structure)
			      Pointer to the board memory map.
			    port (unsigned)
			      The target I/O port number.
			    point (unsigned)
			      The target I/O point number
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

	This module writes a value (0 or 1) to a point of an output port.
	The valid values of "port" are 0 to 1 and "point" are from 0 to 15.
	Otherwise, a -1 is returned.
*/

long wpntdio(c_blk, port, point, value)

struct pmc464 *c_blk;
unsigned port;
unsigned point; 	    /* the I/O point of a port */
unsigned value; 	    /* the output value */

{

/*
    DECLARE LOCAL DATA AREAS:
*/

    unsigned bpos;		/* bit position */
    unsigned nValue;	/* current value of port */
/*
    ENTRY POINT OF ROUTINE
*/

    if (port > 3 || point > 15 || value > 1)	/* error checking */
		return(-1);
    else
    {
		bpos = 1 << point;
		value <<= point;
		nValue = input_word(c_blk->nHandle, (word*)&c_blk->brd_ptr->IOPort[port]);
		output_word(c_blk->nHandle, (word*)&c_blk->brd_ptr->IOPort[port], ( nValue & ~bpos ) | value);
		return(0);
    }
}
