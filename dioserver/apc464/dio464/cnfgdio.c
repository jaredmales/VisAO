//Includes modified by Jared R. Males 6/2/10

#include "../pmccommon/pmcmulticommon.h"
#include "../pmc464.h"

/*
{+D}
    SYSTEM:	Library Software

    FILENAME:	cnfgdio.c

    MODULE
	NAME:	cnfgdio.c - configure 464 board

    VERSION:	A

    CREATION
	DATE:	01/28/03

    CODED BY:	FJM

    ABSTRACT:	This module is used to perform the configure function
		for the pmc464 board.

    CALLING
	SEQUENCE:   cnfgdio(c_blk);
		    where:
			c_blk (pointer to structure)
			    Pointer to the pmc464 structure.

    MODULE TYPE:    void

    I/O RESOURCES:

    SYSTEM
	RESOURCES:

    MODULES
	CALLED:

    REVISIONS:

  DATE	  BY	    PURPOSE
-------  ----	------------------------------------------------

{-D}
*/


/*
    MODULES FUNCTIONAL DETAILS:

    This module is used to perform the "configure board" function
    for the pmc464 board.  A pointer to the Configuration Block will
    be passed to this routine.  The routine will use a pointer
    within the Configuration Block to reference the registers
    on the Board.  Based on flag bits in the Attribute and
    Parameter Flag words in the Configuration Block, the board
    will be configured and various registers will be updated with
    new information which will be transfered from the Configuration
    Block to registers on the Board.
*/



void cnfgdio(c_blk)

struct pmc464 *c_blk;

{

	int i;

/*
    ENTRY POINT OF ROUTINE:
    If the direction Register is to be updated, then update it.
*/

    if(c_blk->dio464ptr->param & DIRECTION)
	{
		output_word(c_blk->nHandle,(word*)&c_blk->brd_ptr->Direction[0], c_blk->dio464ptr->direction[0]);
		output_word(c_blk->nHandle,(word*)&c_blk->brd_ptr->Direction[1], c_blk->dio464ptr->direction[1]);
	}

/*
    If Debounce Duration Registers are to be updated, then update.
*/

    if(c_blk->dio464ptr->param & DEBOUNCE)
	{
		for( i = 0; i < 8; i++)
			output_long(c_blk->nHandle, (long*)&c_blk->brd_ptr->DebounceDurationReg[i], c_blk->dio464ptr->debounce_duration[i]);
	}


/*
    If the Type Select Register is to be updated, then update it.
*/

    if(c_blk->dio464ptr->param & INT_TYPE)
	{
		for( i = 0; i < 4; i++)
			output_word(c_blk->nHandle, (word*)&c_blk->brd_ptr->InterruptTypeReg[i], c_blk->dio464ptr->int_type[i]);
	}

/*
    If the Interrupt Polarity Register is to be updated . . .
*/

    if(c_blk->dio464ptr->param & INT_POLARITY)
	{
		for( i = 0; i < 4; i++)
			output_word(c_blk->nHandle, (word*)&c_blk->brd_ptr->InterruptPolarityReg[i], c_blk->dio464ptr->int_polarity[i]);
	}

/*
    If the Interrupt Status Register is to be updated, then update it.
*/

    if(c_blk->dio464ptr->param & INT_STATUS)
	{
		for( i = 0; i < 4; i++)
			output_word(c_blk->nHandle, (word*)&c_blk->brd_ptr->DInterruptStatusReg[i], c_blk->dio464ptr->int_status[i]);
	}

/*
    If the Interrupt Enable Register is to be updated then do so.
*/

    if(c_blk->dio464ptr->param & INT_ENAB)
	{
		for( i = 0; i < 4; i++)
			output_word(c_blk->nHandle, (word*)&c_blk->brd_ptr->InterruptEnableReg[i], c_blk->dio464ptr->int_enable[i]);
	}
}
