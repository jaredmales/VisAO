//Includes modified by Jared R. Males 6/2/10

#include "../pmccommon/pmcmulticommon.h"
#include "../pmc464.h"

/*
{+D}
    SYSTEM:	Library Software - Pmc464 Digital I/O

    FILENAME:	rstsdio.c

    MODULE
	NAME:	rstsdio - read status of Pmc464 digital

    VERSION:    A

    CREATION
	DATE:	04/25/02

    CODED BY:	FJM

    ABSTRACT:	This module reads status from the Pmc464 digital I/O section.

    CALLING
	SEQUENCE:   rstsdio(c_blk);
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

    This module is used to perform the read status function for the Pmc464
    board.  A pointer to the Status Block will be passed to this routine.
    The routine will use a pointer within the Status Block together with
    offsets to reference the registers on the Board and will transfer the
    status information from the Board to the Status Block.
*/


void rstsdio(c_blk)
struct pmc464 *c_blk;
{

	int i;
/*
    ENTRY POINT OF ROUTINE
*/
    for( i = 0; i < 8; i++)
    {
      c_blk->sdio464ptr->debounce_duration[i] = (unsigned long) input_long(c_blk->nHandle, (long*)&c_blk->brd_ptr->DebounceDurationReg[i]);

      if( i < 4 )
      {
        c_blk->sdio464ptr->int_enable[i] = input_word(c_blk->nHandle, (word*)&c_blk->brd_ptr->InterruptEnableReg[i]);	/* interrupt enable */
	c_blk->sdio464ptr->int_polarity[i] = input_word(c_blk->nHandle, (word*)&c_blk->brd_ptr->InterruptPolarityReg[i]);/* interrupt polarity */	
	c_blk->sdio464ptr->int_type[i] = input_word(c_blk->nHandle, (word*)&c_blk->brd_ptr->InterruptTypeReg[i]);	/* interrupt type*/
      }

      if( i < 2 )
	    c_blk->sdio464ptr->direction[i] = input_word(c_blk->nHandle, (word*)&c_blk->brd_ptr->Direction[i]);
    }			/* direction */
}
