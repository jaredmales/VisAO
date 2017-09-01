//Includes modified by Jared R. Males 6/2/10

#include "../pmccommon/pmcmulticommon.h"
#include "../pmc464.h"

/*
{+D}
    SYSTEM:    Software for Pmc464 Digital I/O

    FILENAME:   drvrdio.c

    MODULE
		NAME:	main - main routine of example software.

    VERSION:    C

    CREATION
		DATE:	01/28/03

    CODED BY:	FJM

    ABSTRACT:	This module is the main routine for the Digital I/O example
		program which demonstrates how the Pmc464 Library is used.

    CALLING
	SEQUENCE:

    MODULE TYPE:    void

    I/O RESOURCES:

    SYSTEM
	RESOURCES:

    MODULES
	CALLED:

    REVISIONS:

  DATE	  BY	    PURPOSE
-------  ----	------------------------------------------------
06/30/06 FJM   Support for multiple apmc464 devices
01/11/07 FJM   Add support x86_64

{-D}
*/


/*
    MODULES FUNCTIONAL DETAILS:

	This module is the main routine for the example program
	which demonstrates how the Pmc464 Digital I/O Library is used.
*/


#ifdef USING_MAIN_MENU
int drvrdio()
#else
int main( void )
#endif
{


/*
    DECLARE LOCAL DATA AREAS:
*/

    char cmd_buff[40];	 /* command line input buffer */
    int item, i;		 /* menu item selection variable */
    int status; 	 /* returned status of driver routines */
    PSTATUS hstatus;	 /* interrupt handler returned status */
    unsigned finished;	 /* flag to exit program */
    long addr;		 /* hold board address */
    long flag;		 /* general flag for exiting loops */
    unsigned point;	 /* I/O point number */
    unsigned port;	 /* I/O port number */
    unsigned val;	 /* value of port or point */
    int hflag;		 /* interrupt handler installed flag */
    struct dio464 dio464;/* digital I/O section pointer */
    struct sdio464 sdio464;/* Status block pointer */
    struct handler_data hdata;/* interrupt handler structure */
    struct pmc464 c_block;

/*
    ENTRY POINT OF ROUTINE:
    INITIALIZATION
*/

    flag = 0;		 /* indicate board address not yet assigned */
    finished = 0;	 /* indicate not finished with program */
    hflag = 0;		 /* indicate interrupt handler not installed yet */

/*
    Initialize the Configuration Parameter Block to default values.
*/
    c_block.bPmc = FALSE;			/* indicate no Pmc initialized and set up yet */
    c_block.bInitialized = FALSE;		/* indicate not ready to talk to Pmc */
    c_block.nHandle = 0;			/* make handle to a closed Pmc board */
    c_block.dio464ptr = &dio464;		/* digital I/O section pointer */
    c_block.sdio464ptr = &sdio464;		/* Status block pointer */

    for( i = 0; i < 8; i++)
    {
	/* configuration structure */
      c_block.dio464ptr->debounce_duration[i] = 0;   /* debounce duration/enable reg */
	/* status structure */
      c_block.sdio464ptr->debounce_duration[i] = 0;   /* debounce duration/enable reg */

      if( i < 4 )
      {
	/* configuration structure */
	c_block.dio464ptr->int_status[i] = 0;		/* pending interrupts to clear */
	c_block.dio464ptr->int_enable[i] = 0;		/* interrupt enable (per bit) */
	c_block.dio464ptr->int_polarity[i] = 0;         /* interrupt polarity */
	c_block.dio464ptr->int_type[i] = 0;		/* interrupt type */
 
	/* status structure */
	c_block.sdio464ptr->int_status[i] = 0;		/* pending interrupts to clear */
	c_block.sdio464ptr->int_enable[i] = 0;		/* interrupt enable (per bit) */
	c_block.sdio464ptr->int_polarity[i] = 0;	/* interrupt polarity */
	c_block.sdio464ptr->int_type[i] = 0;		/* interrupt type */
      }
      if( i < 2 )
      {
	/* configuration structure */
        c_block.dio464ptr->direction[i] = 0;		/* direction */
	/* status structure */
        c_block.sdio464ptr->direction[i] = 0;		/* direction */
      }
    }

    c_block.dio464ptr->param = 0;			/* parameter mask */
  
    hdata.h_pid = getpid();	/* save it in the interrupt handler data structure */
    hdata.hd_ptr = (char *)&c_block; /* put in address of c_block structure also */

/*
	Initialize the Pmc library
*/
    if(InitPmcLib() != S_OK)
    {
	  printf("\nUnable to initialize the Pmc library. Exiting program.\n");
	  exit(0);
    }

/*
	Open an instance of a Pmc device
	Other device instances can be obtained
	by changing parameter 1 of PmcOpen()
*/

    if(PmcOpen(0, &c_block.nHandle, DEVICE_NAME ) != S_OK)
    {
	  printf("\nUnable to Open instance of Pmc464.\n");
	  finished = 1;	 /* indicate finished with program */
    }
    else
    {
      if(PmcInitialize(c_block.nHandle) == S_OK)/* Initialize Pmc */
      {
		GetPmcAddress(c_block.nHandle, &addr);	/* Read back address */
		c_block.brd_ptr = (struct map464 *)addr;
		c_block.bInitialized = TRUE;
		c_block.bPmc = TRUE;
      }
      flag = 1;
    }

/*
    Enter main loop
*/

    while(!finished)
    {
	printf("\n\nPmc464 Digital I/O Demonstration  Rev. B\n\n");
	printf(" 1. Exit this Program\n");
	printf(" 2. Set Up Configuration Block Parameters\n");
	printf(" 3. Configure Board Command\n");
	printf(" 4. Read Status Command\n");
	printf(" 5. Attach Exception Handler\n");
	printf(" 6. Detach Exception Handler\n");
	printf(" 7. Read Input Point\n");
	printf(" 8. Read Input Port\n");
	printf(" 9. Write Output Point\n");
	printf("10. Write Output Port\n");

	printf("\nSelect: ");
	scanf("%d",&item);

/*
    perform the menu item selected.
*/
	switch(item) {
	case 1: /* exit program command */

	    printf("Exit program(y/n)?: ");
	    scanf("%s",cmd_buff);
	    if( cmd_buff[0] == 'y' || cmd_buff[0] == 'Y' )
		finished++;
	break;

	case 2: /* set up configuration block parameters */

	    scfgdio(&c_block);
	break;

	case 3:     /* configure board command */

	    if(!c_block.bInitialized)
		printf("\n>>> ERROR: BOARD ADDRESS NOT SET <<<\n");
	    else
	    {
/*
    Check for pending interrupts and check the "interrupt handler attached"
    flag.  If interrupts are pending or if interrupt handlers are not attached,
    then print an error message.
    If both conditions were false, then go ahead and execute the command.
*/
	      if( input_word (c_block.nHandle, (word*)&c_block.brd_ptr->InterruptRegister ) & 2)
		printf(">>> ERROR: INTERRUPTS ARE PENDING <<<\n");
	      else
	      {
		if(( hflag == 0 ) && (c_block.dio464ptr->param & INT_ENAB)) 
		    printf(">>> ERROR: INTERRUPT HANDLER NOT ATTACHED <<<\n");
		else
		    cnfgdio(&c_block); /* configure the board */
	      }
	    }
	break;

	case 4:     /* read board status command */

	    if(!c_block.bInitialized)
		printf("\n>>> ERROR: BOARD NOT SET UP <<<\n");
	    else
		pstsdio(&c_block);  /* read board */
	break;

	case 5:     /* attach exception handler */
	    if(hflag)
		printf("\n>>> ERROR: HANDLERS ARE ATTACHED <<<\n");
	    else
	    {
		hstatus = 0;
		/* If there were any errors, then print error messages.
		   If no errors, then set "handler attached" flag.*/

		if(hstatus )
	        {
		    printf(">>> ERROR WHEN ATTACHING HANDLER <<<\n");
		    hflag = 0;
		}
		else
		{
		    hstatus = 0;
		    hstatus = EnableInterrupts(c_block.nHandle);
		    if(hstatus != S_OK)
		    {
			printf(">>> ERROR WHEN ENABLING INTERRUPTS <<<\n");
			hflag = 0;
		    }
		    else
		    {
			hflag = 1;
			printf("\nHandlers are now attached\n");
	   	    }
		}
	    } /* end of if/else */
	break;

	case 6: /* detach exception handlers */

	    hflag = 0;
	    DisableInterrupts(c_block.nHandle);
	    printf("\n>>> NO WAY TO DETACH AT THIS TIME <<<n");
	break;

	case 7: /* Read Digital Input Point */

	    if(!c_block.bInitialized)
		printf("\n>>> ERROR: BOARD ADDRESS NOT SET <<<\n");
	    else
	    {
		printf("\nEnter Input port number   (0 - 3): ");
		scanf("%d",&port);
		printf("\nEnter Input point number (0 - 15): ");
		scanf("%d",&point);
		status = rpntdio(&c_block,port,point);
		if(status == -1)
		    printf("\n>>> ERROR: PARAMETER OUT OF RANGE <<<\n");
		else
		    printf("\nValue of point %d: %X\n",point,status);
	    }
	break;

	case 8: /* Read Digital Input Port */

	    if(!c_block.bInitialized)
		printf("\n>>> ERROR: BOARD ADDRESS NOT SET <<<\n");
	    else
	    {
		printf("\nEnter port number   (0 - 3): ");
		scanf("%d",&port);
		status = rprtdio(&c_block,port);
		if(status == -1)
		    printf("\n>>> ERROR: PARAMETER OUT OF RANGE <<<\n");
		else
		    printf("\nPort %X Value = %X\n",port,status);
	    }
	break;


	case 9: /* Write Digital Point */

	    if(!c_block.bInitialized)
		printf("\n>>> ERROR: BOARD ADDRESS NOT SET <<<\n");
	    else
	    {
		printf("\nEnter port number   (0 - 3): ");
		scanf("%d",&port);
		printf("\nEnter I/O point number  (0 - 15): ");
		scanf("%d",&point);
		printf("\nEnter point value (0 - 1): ");
		scanf("%x",&val);
		status = wpntdio(&c_block,port,point,val);
		if(status == -1)
		    printf("\n>>> ERROR: PARAMETER OUT OF RANGE <<<\n");
	    }
	break;

	case 10: /* Write Digital Port */

	    if(!c_block.bInitialized)
		printf("\n>>> ERROR: BOARD ADDRESS NOT SET <<<\n");
	    else
	    {
		printf("\nEnter port number   (0 - 3): ");
		scanf("%d",&port);
		printf("\nEnter output value in hex (0000 - FFFF): ");
		scanf("%x",&val);
		status = wprtdio(&c_block,port,val);
		if(status == -1)
		    printf("\n>>> ERROR: PARAMETER OUT OF RANGE <<<\n");
	    }
	break;
	}   /* end of switch */
    }	/* end of while */

/*    disable interrupts from Pmc module */
    if(c_block.bInitialized)	  /* module address was set */
    {
      c_block.dio464ptr->param = (INT_STATUS | INT_ENAB);        /* parameter mask */
      for( i = 0; i < 4; i++)
      {
         c_block.dio464ptr->int_status[i] = 0xffff; /* pending interrupts to clear */
         c_block.dio464ptr->int_enable[i] = 0;      /* interrupt enable (per bit) */
      }
      cnfgdio(&c_block);                      /* configure the board */
    }
    DisableInterrupts(c_block.nHandle);
    if(c_block.bPmc == TRUE)
	  PmcClose(c_block.nHandle);

    printf("\nEXIT PROGRAM\n");

}   /* end of main */



/*
{+D}
    SYSTEM:	    Software

    FILENAME:	    drvr464.c

    MODULE NAME:    get_dio_param - get a parameter from the console

    VERSION:	    A

    CREATION DATE:  06/06/01

    DESIGNED BY:    RTL

    CODED BY:	    RTL

    ABSTRACT:	    Routine which is used to get parameters

    CALLING
	SEQUENCE:   get_dio_param();

    MODULE TYPE:    long

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


long get_dio_param()
{

/*
    declare local storage.
*/

    int temp;

    printf("enter hex parameter: ");
    scanf("%x",&temp);
    printf("\n");
    return((long)temp);
}



/*
{+D}
    SYSTEM:	    Software

    FILENAME:	    drvr464.c

    MODULE NAME:    scfgdio - set configuration block contents.

    VERSION:	    A

    CREATION DATE:  06/06/01

    CODED BY:	    FJM

    ABSTRACT:	    Routine which is used to enter parameters into
				    the Configuration Block.

    CALLING
	SEQUENCE:   scfgdio(ptr)
		    where:
				ptr (pointer to structure)
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
*/

void scfgdio( ptr )

struct pmc464 *ptr;
{

/*
    DECLARE LOCAL DATA AREAS:
*/
    int item, i, j;			    /* menu item variable */
    unsigned finished,  finished2;		    /* flags to exit loops */

/*
    ENTRY POINT OF ROUTINE:
*/
    finished = 0;
    while(!finished)
    {
	printf("\n\nConfiguration Block Parameters\n\n");
	printf(" 1. Return to Previous Menu\n");
	printf(" 2. Board Pointer:     %lX\n",(unsigned long)ptr->brd_ptr);
	printf(" 3. Parameter Mask:    %02X\n",ptr->dio464ptr->param);
	printf(" 4. Data Direction:    ");
    for( j = 0; j < 2; j++)
	  printf("%X = %04X  ",j,ptr->dio464ptr->direction[j]);

	printf("\n 5. Int. Status:       ");
    for( j = 0; j < 4; j++)
	  printf("%X = %04X  ",j,ptr->dio464ptr->int_status[j]);

	printf("\n 6. Int. Enable:       ");
    for( j = 0; j < 4; j++)
	  printf("%X = %04X  ",j,ptr->dio464ptr->int_enable[j]);

	printf("\n 7. Int. Polarity:     ");
    for( j = 0; j < 4; j++)
	  printf("%X = %04X  ",j,ptr->dio464ptr->int_polarity[j]);

	printf("\n 8. Int. Type:         ");
    for( j = 0; j < 4; j++)
	  printf("%X = %04X  ",j,ptr->dio464ptr->int_type[j]);

	printf("\n 9. Debounce Duration:\n");
    for( j = 0; j < 4; j++)
          printf("    %X = %08X",j,ptr->dio464ptr->debounce_duration[j]);
	printf("\n");
    for( j = 4; j < 8; j++)
          printf("    %X = %08X",j,ptr->dio464ptr->debounce_duration[j]);

	printf("\nSelect: ");
	scanf("%d",&item);

	switch(item){

	case 1: /* return to previous menu */
	    finished++;
	break;

	case 2: /* board address */
	    printf("BOARD ADDRESS CAN NOT BE CHANGED\n");
	break;

	case 3: /* parameter mask */
		ptr->dio464ptr->param = (word)get_dio_param();
	break;

	case 4: /* direction */
		printf("\nEnter word (0 or 1): ");
		scanf("%x",&j);
		ptr->dio464ptr->direction[j&1] = (word)get_dio_param();
	break;
	
	case 5: /* interrupt status register */
		printf("\nEnter port number   (0 - 3): ");
		scanf("%x",&j);
		ptr->dio464ptr->int_status[j&3] = (word)get_dio_param();
	break;

	case 6: /* interrupt enable reg. */
		printf("\nEnter port number   (0 - 3): ");
		scanf("%x",&j);
                ptr->dio464ptr->int_enable[j&3] = (word)get_dio_param();
	break;

	case 7: /* interrupt polarity reg. */
		printf("\nEnter port number   (0 - 3): ");
		scanf("%x",&j);
		ptr->dio464ptr->int_polarity[j&3] = (word)get_dio_param();
	break;

	case 8: /* interrupt type reg. */
		printf("\nEnter port number   (0 - 3): ");
		scanf("%x",&j);
	        ptr->dio464ptr->int_type[j&3] = (word)get_dio_param();
	break;

	case 9: /* debounce duration reg. */
		printf("\nEnter register number   (0 - 7): ");
		scanf("%x",&j);
	        ptr->dio464ptr->debounce_duration[j&7] = (unsigned long)get_dio_param();
	break;
	}
    }
}



/*
{+D}
    SYSTEM:	    Software

    FILENAME:	    drvr408.c

    MODULE NAME:    pstsdio - print board status information

    VERSION:	    A

    CREATION DATE:  6/6/01

    DESIGNED BY:    FJM

    CODED BY:	    FJM

    ABSTRACT:	    Routine which is used to cause the "Read Board Status"
				    command to be executed and to print out the results to
				    the console.

    CALLING
	SEQUENCE:   pstsdio(ptr)
		    where:
				ptr (pointer to structure)
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

*/

void pstsdio(ptr)
struct pmc464 *ptr;
{

/*
	DECLARE LOCAL DATA AREAS:
*/
	int item, j;	    /* menu item variable */
	unsigned finished;  /* flags to exit loops */
/*
	ENTRY POINT OF ROUTINE:
*/

	finished = 0;
	while(!finished)
	{
	  rstsdio(ptr);	   /* Read Command */
	  printf("\n\nBoard Status Information");
  	  printf("\nDirection Register:          ");
      for( j = 0; j < 2; j++)
		  printf("%X = %04X  ",j,ptr->sdio464ptr->direction[j]);

	  printf("\nInterrupt Status Register:   ");
      for( j = 0; j < 4; j++)
		  printf("%X = %04X  ",j,ptr->sdio464ptr->int_status[j]);

	  printf("\nInterrupt Enable Register:   ");
      for( j = 0; j < 4; j++)
		  printf("%X = %04X  ",j,ptr->sdio464ptr->int_enable[j]);

	  printf("\nInterrupt Polarity Register: ");
      for( j = 0; j < 4; j++)
		  printf("%X = %04X  ",j,ptr->sdio464ptr->int_polarity[j]);

	  printf("\nInterrupt Type Register:     ");
      for( j = 0; j < 4; j++)
		  printf("%X = %04X  ",j,ptr->sdio464ptr->int_type[j]);

	printf("\nDebounce Duration Registers:   \n");
    for( j = 0; j < 4; j++)
          printf("    %X = %08X",j,ptr->dio464ptr->debounce_duration[j]);
	printf("\n");
    for( j = 4; j < 8; j++)
          printf("    %X = %08X",j,ptr->dio464ptr->debounce_duration[j]);

	  printf("\n\nRead Board Status\n");
	  printf(" 1. Return to Previous Menu\n");
	  printf(" 2. Read Status Again\n");
	  printf("\nselect: ");
	  scanf("%d",&item);

	  switch(item)
	  {
	    case 1: /* return to previous menu */
	      finished++;
	    break;
	  }
	}
}
