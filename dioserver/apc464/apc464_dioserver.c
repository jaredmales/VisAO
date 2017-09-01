#include "apc464_dioserver.h"

int init_apc464_info(apc464_info * apc464)
{
   /*apc464 Library stuff:*/
   /*
      ENTRY POINT OF ROUTINE:
      INITIALIZATION
   */

   apc464->flag = 0;       /* indicate board address not yet assigned */
   apc464->finished = 0;   /* indicate not finished with program */
   apc464->hflag = 0;      /* indicate interrupt handler not installed yet */
   
   return 0;
}

int apc464_init(void * diocard_info)
{
   apc464_info * apc464 = (apc464_info *) diocard_info;

   int i;
     
   /*
      Initialize the Configuration Parameter Block to default values.
   */
   apc464->c_block.bPmc = FALSE;			/* indicate no Pmc initialized and set up yet */
   apc464->c_block.bInitialized = FALSE;		/* indicate not ready to talk to Pmc */
   apc464->c_block.nHandle = 0;			/* make handle to a closed Pmc board */
   apc464->c_block.dio464ptr = &apc464->dio464;		/* digital I/O section pointer */
   apc464->c_block.sdio464ptr = &apc464->sdio464;		/* Status block pointer */

    for( i = 0; i < 8; i++)
    {
      /* configuration structure */
      apc464->c_block.dio464ptr->debounce_duration[i] = 0;   /* debounce duration/enable reg */
      /* status structure */
      apc464->c_block.sdio464ptr->debounce_duration[i] = 0;   /* debounce duration/enable reg */

      if( i < 4 )
      {
         /* configuration structure */
         apc464->c_block.dio464ptr->int_status[i] = 0;		/* pending interrupts to clear */
         apc464->c_block.dio464ptr->int_enable[i] = 0;		/* interrupt enable (per bit) */
         apc464->c_block.dio464ptr->int_polarity[i] = 0;         /* interrupt polarity */
         apc464->c_block.dio464ptr->int_type[i] = 0;		/* interrupt type */

         /* status structure */
         apc464->c_block.sdio464ptr->int_status[i] = 0;		/* pending interrupts to clear */
         apc464->c_block.sdio464ptr->int_enable[i] = 0;		/* interrupt enable (per bit) */
         apc464->c_block.sdio464ptr->int_polarity[i] = 0;	/* interrupt polarity */
         apc464->c_block.sdio464ptr->int_type[i] = 0;		/* interrupt type */
      }
      if( i < 2 )
      {
         /* configuration structure */
         apc464->c_block.dio464ptr->direction[i] = 0;		/* direction */
         /* status structure */
        apc464->c_block.sdio464ptr->direction[i] = 0;		/* direction */
      }
   }

   apc464->c_block.dio464ptr->param = 0;			/* parameter mask */

   apc464->hdata.h_pid = getpid();	/* save it in the interrupt handler data structure */
   apc464->hdata.hd_ptr = (char *)&apc464->c_block; /* put in address of c_block structure also */

   /*
      Initialize the Pmc library
   */
   if(InitPmcLib() != S_OK)
   {
      fprintf(stderr,"\nUnable to initialize the Pmc library.\n");
      return -1;
   }

   /*
      Open an instance of a Pmc device
      Other device instances can be obtained
      by changing parameter 1 of PmcOpen()
   */

   if(PmcOpen(0, &apc464->c_block.nHandle, DEVICE_NAME ) != S_OK)
   {
      fprintf(stderr, "\nUnable to Open instance of Pmc464.\n");
      apc464->finished = 1;	 /* indicate finished with program */
      return -1;
   }
   else
   {
      if(PmcInitialize(apc464->c_block.nHandle) == S_OK)/* Initialize Pmc */
      {
         GetPmcAddress(apc464->c_block.nHandle, &apc464->addr);	/* Read back address */
         apc464->c_block.brd_ptr = (struct map464 *)apc464->addr;
         apc464->c_block.bInitialized = TRUE;
         apc464->c_block.bPmc = TRUE;
      }
      apc464->flag = 1;
   }

   //*******************************************************
   //scfgdio(&c_block);
   apc464->c_block.dio464ptr->param = (word) 16;
   apc464->c_block.dio464ptr->direction[0] = (word)0x01;
   apc464->c_block.dio464ptr->direction[1] = (word)0;
   if(!apc464->c_block.bInitialized) 
   {  
      fprintf(stderr, "\n>>> ERROR: BOARD ADDRESS NOT SET <<<\n");
      return -1;
   }
   else
   {
      /*
      Check for pending interrupts and check the "interrupt handler attached"
      flag.  If interrupts are pending or if interrupt handlers are not attached,
      then print an error message.
      If both conditions were false, then go ahead and execute the command.
      */
      if( input_word (apc464->c_block.nHandle, (word*)&apc464->c_block.brd_ptr->InterruptRegister ) & 2) 
      {
         fprintf(stderr, ">>> ERROR: INTERRUPTS ARE PENDING <<<\n");
         return -1;
      }
      else
      {
         if(( apc464->hflag == 0 ) && (apc464->c_block.dio464ptr->param & INT_ENAB)) 
         {
            fprintf(stderr, ">>> ERROR: INTERRUPT HANDLER NOT ATTACHED <<<\n");
            return -1;
         }
         else cnfgdio(&apc464->c_block); /* configure the board */
      }
   }

   return 0;
}

int apc464_write(void * diocard_info, int ch, int xput)
{
   apc464_info * apc464 = (apc464_info *) diocard_info;

   wpntdio(&apc464->c_block,0,ch,xput);
   return 0;
}

int apc464_read(void * diocard_info, int ch)
{
   apc464_info * apc464 = (apc464_info *) diocard_info;

   return rpntdio(&apc464->c_block,0,ch);
}

