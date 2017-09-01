
/*
{+D}
    SYSTEM:		Linux

    FILENAME:		PmcMultiCommon.c

    MODULE NAME:	Functions common to the Linux example software.

    VERSION:		D

    CREATION DATE:	06/29/06

    DESIGNED BY:	FJM

    CODED BY:		FJM

    ABSTRACT:		This file contains the implementation of the functions for
			Acromag PMC modules.

    CALLING
	SEQUENCE:

    MODULE TYPE:

    I/O RESOURCES:

    SYSTEM
	RESOURCES:

    MODULES
	CALLED:

    REVISIONS:

  DATE	  BY	    PURPOSE
-------  ----	------------------------------------------------
12/19/07  FJM   Fixed close() bug needs handle not device name
07/21/08  FJM   Fixed PmcInitialize() bug touched member with invalid pointer
04/01/09  FJM   Add blocking_start_convert for ain boards

{-D}
*/

/*
	This file contains the implementation of the functions for
	Acromag PMC modules.
*/

#include "pmcmulticommon.h"

/*	Global variables */
int	gNumberPmcs = -1;		/* Number of Pmc boards that have been opened and/or flag = -1...
                                           if library is uninitialized see function InitPmcLib() */


PMCDATA_STRUCT *gpPmcs[MAX_PMCS];	/* pointer to the Pmc boards */



/*
        Some systems can resolve BIG_ENDIAN/LITTLE_ENDIAN data transfers in hardware.
        If the system is resolving BIG_ENDIAN/LITTLE_ENDIAN data transfers in hardware
        the SWAP_ENDIAN define should be commented out.

        When resolving the BIG_ENDIAN/LITTLE_ENDIAN data transfers in hardware is not
        possible or desired the SWAP_ENDIAN define is provided.

        Define SWAP_ENDIAN to enable software byte swapping for word and long transfers
*/

/* #define SWAP_ENDIAN		/ * SWAP_ENDIAN enables software byte swapping for word and long transfers */


word SwapBytes( word v )             
{
#ifdef SWAP_ENDIAN		/* endian correction if needed */
  word  Swapped;

  Swapped = v << 8;
  Swapped |= ( v >> 8 );
  return( Swapped );
#else				/* no endian correction needed */
  return( v );
#endif /* SWAP_ENDIAN */

}


long SwapLong( long v )             
{
#ifdef SWAP_ENDIAN		/* endian correction if needed */
 word Swap1, Swap2;
 long Swapped; 

  Swap1 = (word)(v >> 16);
  Swap1 = SwapBytes( Swap1 );             

  Swap2 = (word)v & 0xffff;
  Swap2 = SwapBytes( Swap2 );

  Swapped = (long)(Swap2 << 16); 
  Swapped |= (long)(Swap1 & 0xffff);
  return( Swapped );
#else				/* no endian correction needed */
  return( v );
#endif /* SWAP_ENDIAN */
 
}



byte input_byte(int nHandle, byte *p)
{
	PMCDATA_STRUCT* pPmc;	/*  local */
	unsigned long data[2];

	pPmc = GetPmc(nHandle);
	if(pPmc == NULL)
		return((byte)0);

	if( p )
	{
           /* place address to read byte from in data [0]; */
           data[0] = (unsigned long) p;
           data[1] = (unsigned long) 0;
           /* pram3 = function: 1=read8bits,2=read16bits,4=read32bits */
           read( pPmc->nPmcDeviceHandle, &data[0], 1 );
           return( (byte)data[1] );
	}
	return((byte)0);
}

word input_word(int nHandle, word *p)
{
	PMCDATA_STRUCT* pPmc;	/*  local */
	unsigned long data[2];

	pPmc = GetPmc(nHandle);
	if(pPmc == NULL)
		return((word)0);

	if( p )
	{
           /* place address to read word from in data [0]; */
           data[0] = (unsigned long) p;
           /* pram3 = function: 1=read8bits,2=read16bits,4=read32bits */
           read( pPmc->nPmcDeviceHandle, &data[0], 2 );
           return(  SwapBytes( (word)data[1] ) );
	}
	return((word)0);
}

long input_long(int nHandle, long *p)
{
	PMCDATA_STRUCT* pPmc;	/*  local */
	unsigned long data[2];

	pPmc = GetPmc(nHandle);
	if(pPmc == NULL)
		return((long)0);

	if( p )
	{
           /* place address to read word from in data [0]; */
           data[0] = (unsigned long) p;
           /* pram3 = function: 1=read8bits,2=read16bits,4=read32bits */
           read( pPmc->nPmcDeviceHandle, &data[0], 4 );
           return(  SwapLong( (long)data[1] ) );
	}
	return((long)0);
}


void output_byte(int nHandle, byte *p, byte v)
{
	PMCDATA_STRUCT* pPmc;	/*  local */
	unsigned long data[2];

	pPmc = GetPmc(nHandle);
	if(pPmc == NULL)
		return;

	if( p )
	{
		/* place address to write byte in data [0]; */
		data[0] = (unsigned long) p;
		/* place value to write @ address data [1]; */
		data[1] = (unsigned long) v;
	        /* pram3 = function: 1=write8bits,2=write16bits,4=write32bits */
		write( pPmc->nPmcDeviceHandle, &data[0], 1 );
	}
}

void output_word(int nHandle, word *p, word v)
{
	PMCDATA_STRUCT* pPmc;	/*  local */
	unsigned long data[2];

	pPmc = GetPmc(nHandle);
	if(pPmc == NULL)
		return;

	if( p )
	{
           /* place address to write word in data [0]; */
           data[0] = (unsigned long) p;
           /* place value to write @ address data [1]; */
           data[1] = (unsigned long) SwapBytes( v );
           /* pram3 = function: 1=write8bits,2=write16bits,4=write32bits */
           write( pPmc->nPmcDeviceHandle, &data[0], 2 );
	}
}

void blocking_start_convert(int nHandle, word *p, word v)
{
	PMCDATA_STRUCT* pPmc;	/*  local carrier */
	unsigned long data[2];

	pPmc = GetPmc(nHandle);
	if(pPmc == NULL)
		return;

	if( p )
	{
           /* place address to write word in data [0]; */
           data[0] = (unsigned long) p;
           /* place value to write @ address data [1]; */
           data[1] = (unsigned long) SwapBytes( v );
           /* pram3 = function: 8=blocking_start_convert */
           write( pPmc->nPmcDeviceHandle, &data[0], 8 );
	}
}

void output_long(int nHandle, long *p, long v)
{
	PMCDATA_STRUCT* pPmc;	/*  local */
	unsigned long data[2];

	pPmc = GetPmc(nHandle);
	if(pPmc == NULL)
		return;

	if( p )
	{
           /* place address to write word in data [0]; */
           data[0] = (unsigned long) p;
           /* place value to write @ address data [1]; */
           data[1] = (unsigned long) SwapLong( v );
           /* pram3 = function: 1=write8bits,2=write16bits,4=write32bits */
           write( pPmc->nPmcDeviceHandle, &data[0], 4 );
	}
}


PSTATUS GetPmcAddress(int nHandle, long* pAddress)
{
	PMCDATA_STRUCT* pPmc;

	pPmc = GetPmc(nHandle);
	if(pPmc == 0)
		return E_INVALID_HANDLE;

	*pAddress = pPmc->lBaseAddress;
	return (PSTATUS)S_OK;
}


PSTATUS EnableInterrupts(int nHandle)
{
	PMCDATA_STRUCT* pPmc;
	PMC_BOARD_MEMORY_MAP* pPmcCard;
	PSTATUS status;
	word nValue;	/* new */

	pPmc = GetPmc(nHandle);
	if(pPmc == 0)
		return E_INVALID_HANDLE;

	if(pPmc->bInitialized == FALSE)
		return E_NOT_INITIALIZED;
		
	/*  Clear any pending interrupt */
	pPmcCard = (PMC_BOARD_MEMORY_MAP*)pPmc->lBaseAddress;
	nValue = input_word( nHandle,(word*)&pPmcCard->InterruptRegister);
	output_word( nHandle, (word*)&pPmcCard->InterruptRegister, ( nValue | APMC_INT_RELEASE )); 

	/* do not attach handler for invalid interrupt level */
	if(0 > pPmc->nIntLevel || pPmc->nIntLevel > 254)
		return E_NO_INTERRUPTS;
		
	/* Enable interrupts */
	nValue = input_word( nHandle, (word*)&pPmcCard->InterruptRegister);
	output_word( nHandle, (word*)&pPmcCard->InterruptRegister, ( nValue | APMC_INT_ENABLE ));
	pPmc->bIntEnabled = TRUE;	/* mark interrupts enabled */
	return (PSTATUS)S_OK;
}


PSTATUS DisableInterrupts(int nHandle)
{
	PMCDATA_STRUCT* pPmc;
	PMC_BOARD_MEMORY_MAP* pPmcCard;
	word nValue; /* new */

	pPmc = GetPmc(nHandle);
	if(pPmc == 0)
		return E_INVALID_HANDLE;

	if(pPmc->bInitialized == FALSE)
		return E_NOT_INITIALIZED;

	/* Disable interrupt */
	pPmcCard = (PMC_BOARD_MEMORY_MAP*)pPmc->lBaseAddress;
	nValue = input_word( nHandle, (word*)&pPmcCard->InterruptRegister);
	nValue &= ~APMC_INT_ENABLE;
	output_word( nHandle, (word*)&pPmcCard->InterruptRegister, nValue); 
	pPmc->bIntEnabled = FALSE;
	return (PSTATUS)S_OK;
}



PSTATUS InitPmcLib(void)
{
	int i;				/* General purpose index */

        if( gNumberPmcs == -1)		/* first time used - initialize pointers to 0 */
        {
	  gNumberPmcs = 0;		/* Initialize number of Pmcs to 0 */

	  /* initialize the pointers to the Pmc data structure */
	  for(i = 0; i < MAX_PMCS; i++)
		gpPmcs[i] = 0;		/* set to a NULL pointer */
        }
	return (PSTATUS)S_OK;
}


PSTATUS PmcOpen(int nDevInstance, int* pHandle, char* devname)
{
	PMCDATA_STRUCT* pPmc;		/* local pointer */
        unsigned long data[MAX_PMCS];
	char devnamebuf[64];
	char devnumbuf[8];

	*pHandle = -1;		/* set callers handle to an invalid value */

	if(gNumberPmcs == MAX_PMCS)
		return E_OUT_OF_PMCS;

	/* Allocate memory for a new Pmc structure */
	pPmc = (PMCDATA_STRUCT*)malloc(sizeof(PMCDATA_STRUCT));

	if(pPmc == 0)
		return (PSTATUS)E_OUT_OF_MEMORY;

	pPmc->nHandle = -1;
	pPmc->bInitialized = FALSE;
	pPmc->bIntEnabled = FALSE;
	pPmc->nPmcDeviceHandle = 0;
	pPmc->lBaseAddress = 0;
	pPmc->nInteruptID = 0;
	pPmc->nIntLevel = 0;

        memset( &pPmc->devname[0], 0, sizeof(pPmc->devname));
        memset( &devnamebuf[0], 0, sizeof(devnamebuf));
        memset( &devnumbuf[0], 0, sizeof(devnumbuf));

        strcpy(devnamebuf, "/dev/");
        strcat(devnamebuf, devname);
        sprintf(&devnumbuf[0],"%d",nDevInstance);
        strcat(devnamebuf, devnumbuf);

	pPmc->nPmcDeviceHandle = open( devnamebuf, O_RDWR );

	if( pPmc->nPmcDeviceHandle < 0 )
        {
        	free((void*)pPmc);		/* delete the memory for this Pmc */
		return (PSTATUS)ERROR;
        }
	strcpy(&pPmc->devname[0], &devnamebuf[0]);	/* save device name */

	/* Get Base Address of Pmc */
        memset( &data[0], 0, sizeof(data)); /* no mem if data[x] returns 0 from ioctl() */
	ioctl( pPmc->nPmcDeviceHandle, 5, &data[0] );		/* get address cmd */
	pPmc->lBaseAddress = data[nDevInstance];

	/* Get IRQ Number from Pmc */
	ioctl( pPmc->nPmcDeviceHandle, 6, &data[0] );		/* get IRQ cmd */
	pPmc->nIntLevel = ( int )( data[nDevInstance] & 0xFF );

	AddPmc(pPmc);                  /* call function to add Pmc to array and set handle */
	*pHandle = pPmc->nHandle;      /* return our addpmc handle */

	return (PSTATUS)S_OK;
}


PSTATUS PmcClose(int nHandle)
{
	/*  Delete the PMC with the provided handle */
	PMCDATA_STRUCT* pPmc;	/* local pmc pointer */

	pPmc = GetPmc(nHandle);

	if(pPmc == 0)
		return E_INVALID_HANDLE;

	if(pPmc->bInitialized == FALSE)
		return E_NOT_INITIALIZED;

  	close( pPmc->nPmcDeviceHandle );

  	pPmc->nPmcDeviceHandle = -1;
	DeletePmc(nHandle);		/*  Delete the Pmc with the provided handle */

	return (PSTATUS)S_OK;
}


PSTATUS PmcInitialize(int nHandle)
{
	PMCDATA_STRUCT* pPmc;
	PMC_BOARD_MEMORY_MAP* pPmcCard;

	pPmc = GetPmc(nHandle);

	if(pPmc == 0)
		return E_INVALID_HANDLE;

	/*  initialize Pmc */
	pPmcCard = (PMC_BOARD_MEMORY_MAP*)pPmc->lBaseAddress;
	output_word( nHandle, (word*)&pPmcCard->InterruptRegister, (word)APMC_INT_RELEASE);
	pPmc->bInitialized = TRUE;	/* Pmc is now initialized */

	return (PSTATUS)S_OK;
}


void AddPmc(PMCDATA_STRUCT* pPmc)
{
	int i, j;			/* general purpose index */
	BOOL bFound;			/* general purpose BOOL */

	for(i = 0; i < MAX_PMCS; i++)	/* Determine a handle for this Pmc */
	{
		bFound = TRUE;
		for(j = 0; j < gNumberPmcs; j++)
		{
			if(i == gpPmcs[j]->nHandle)
			{
				bFound = FALSE;
				break;
			}
		}

		if(bFound)
			break;
	}

	pPmc->nHandle = i;             	/* set new handle */
	gpPmcs[gNumberPmcs] = pPmc;	/* add Pmc to array */
	gNumberPmcs++;			/* increment number of Pmcs */
}


void DeletePmc(int nHandle)
{
	PMCDATA_STRUCT* pPmc;
	int i;

	if(gNumberPmcs == 0)
		return;

	pPmc = 0;			/* initialize pointer to null */
	for(i = 0; i < gNumberPmcs; i++)/* Find Pmc that has this handle */
	{
		if(nHandle == gpPmcs[i]->nHandle)
		{
			pPmc = gpPmcs[i];
			break;
		}
	}
	if(pPmc == 0)			/* return if no Pmc has been found */
		return;

	free((void*)pPmc);		/* delete the memory for this Pmc */

	/* Rearrange Pmc array */
	gpPmcs[i] = gpPmcs[gNumberPmcs - 1];
	gpPmcs[gNumberPmcs - 1] = 0;
	gNumberPmcs--;			/* decrement Pmc count */
}


PMCDATA_STRUCT* GetPmc(int nHandle)
{
	PMCDATA_STRUCT* pPmc;
	int i;				/* General purpose index */

	for(i = 0; i < gNumberPmcs; i++)/* Find Pmc that has this handle */
	{
		if(nHandle == gpPmcs[i]->nHandle)
		{
			pPmc = gpPmcs[i];
			return pPmc;
		}
	}
	return (PMCDATA_STRUCT*)0;	/* return null */
}
