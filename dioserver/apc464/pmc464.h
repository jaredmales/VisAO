
#ifdef __cplusplus
extern "C" {
#endif

#define USING_MAIN_MENU


/*
{+D}
    SYSTEM:         Software for PMC464
    
    FILE NAME:      pmc464.h

    VERSION:	    C

    CREATION DATE:  01/27/03

    DESIGNED BY:    F.M.

    CODED BY:	    F.M.

    ABSTRACT:	    This module contains the definitions and structures
                    used by the PMC464 library.

    CALLING
	SEQUENCE:

    MODULE TYPE:    header file

    I/O RESOURCES:

    SYSTEM
	RESOURCES:

    MODULES
	CALLED:

    REVISIONS:

DATE	 BY	PURPOSE
-------  ----	------------------------------------------------
06/30/06 FJM   Support for multiple apmc464 devices
01/11/07 FJM   Add support x86_64

{-D}
*/


/*
    MODULES FUNCTIONAL DETAILS:

    This module contains the definitions and structures used by
    the PMC464 library.
*/

/*
    DEFINITIONS:
*/


#define DEVICE_NAME	"apmc464_"	/* the name of the device */


typedef enum
{
	Success = 0,
	ParameterOutOfRange = 1,	/* Parameter in error */
	InvalidPointer = 2,		/* Flag NULL pointers */
	DataDirection = 3,		/* data direction error */
	TimeOut = 4			/* time-out error */
} PMCSTAT;



/*
    Parameter mask bit positions
*/

#define INT_STATUS     1	/* interrupt status register */
#define INT_ENAB       2	/* interrupt enable register */
#define INT_POLARITY   4	/* interrupt polarity register */
#define INT_TYPE       8	/* interrupt type register */
#define DIRECTION      0x10	/* direction register */
#define DEBOUNCE       0x20	/* debounce registers */


struct map464		   /* Memory map of the digital I/O board section */
{
	/* Digital I/O section */
	ushort InterruptRegister;
	ushort unusedw1;
	ushort CInterruptStatusReg;
	ushort unusedw2;
	ushort DInterruptStatusReg[4];
	ushort IOPort[4];
	ushort Direction[2];
	ushort InterruptEnableReg[4];
	ushort InterruptTypeReg[4];
	ushort InterruptPolarityReg[4];
	ushort CounterTrigger;
	ushort CounterStop;
	uint32_t CounterControl1;
	uint32_t CounterControl2;
	uint32_t CounterControl3;
	uint32_t CounterControl4;
	uint32_t CounterReadBack1;
	uint32_t CounterReadBack2;
	uint32_t CounterConstantA1;
	uint32_t CounterConstantA2;
	uint32_t CounterConstantB1;
	uint32_t CounterConstantB2;
	uint32_t DebounceDurationReg[8];
};


/*
    Defined below is the structure which is used to hold the
    board's status information.
*/

struct sdio464
{
    ushort direction[2];		/* data direction register */
    ushort int_enable[4];		/* interrupt enable register */
    ushort int_status[4];		/* interrupt status register */
    ushort int_type[4];			/* interrupt type select register */
    ushort int_polarity[4];		/* interrupt input polarity register */
    uint32_t debounce_duration[8];	/* debounce_duration registers */
};

/*
    Defined below is the structure which is used to hold the
    Digital I/O section of the board's configuration information.
*/

struct dio464
{
    ushort param;			/* parameter mask for configuring board */
    ushort direction[2];		/* data direction register */
    ushort int_enable[4];		/* interrupt enable register */
    ushort int_status[4];		/* interrupt status register */
    ushort int_type[4];			/* interrupt type select register */
    ushort int_polarity[4];		/* interrupt input polarity register */
    uint32_t debounce_duration[8];	/* debounce_duration registers */
};


/*
	Mode and option selection enumerations
*/

enum
{
	None	  = 0,          /* disable counter */
	PulseWidthMod = 2,      /* pulse width modulation */
	Watchdog  = 3,          /* watchdog function */
	EventCounting   = 4,    /* event counting/frequency measurement */
	InPulseWidthMeasure = 5,/* input pulse width measurement */
	InPeriodMeasure  = 6,   /* input period measurement */
	OneShot   = 7,          /* One-Shot output pluse */


	OutPolLow = 0,          /* output polarity active low */
	OutPolHi  = 1,          /* output polarity active high */

	InABCPolDisabled = 0,	/* disabled */
	InABCPolLow  = 1,       /* input polarity active low */
	InABCPolHi   = 2,       /* input polarity active high */
	InABCUpDown  = 3,       /* up/down */
	InABCGate_off  = 3,     /* gate off */
	InAX4  = 3,

	InC1_25Mhz   = 0,       /* internal 1.25MHZ clock */
	InC2_5Mhz    = 1,       /* internal 2.5MHZ clock */
	InC5Mhz      = 2,       /* internal 5MHZ clock */
	InC10Mhz     = 3,       /* internal 10MHZ clock */
	InC20Mhz     = 4,       /* internal 20MHZ clock */
	ExClock      = 5,       /* external clock */

	DebounceOff =  0,	/* Debounce disabled */
	Debounce5_6 =  1,
	Debounce50_4=  2,
	Debounce408_8= 3,
	Debounce3_276= 4,

	CtrSize16 = 0,          /* counter 16 bit */
	CtrSize32 = 1,          /* counter 32 bit */

	IntDisable= 0,          /* disable interrupt */
	IntEnable = 1           /* interrupt enabled */
};



struct ct464
 {
    BYTE m_Mode[5];             /* the counter mode */
    BOOL m_OutputPolarity[5];	/* output polarity */
    BYTE m_InputAPolarity[5];	/* input polarity A */
    BYTE m_InputBPolarity[5];	/* input polarity B */
    BYTE m_InputCPolarity[5];	/* input polarity C */
    BYTE m_ClockSource[5];      /* clock source */
    word m_Debounce[5];         /* debounce */
    BOOL m_CounterSize[5];      /* 16/32 bit counter size flags */
    BOOL m_InterruptEnable[5];	/* interrupt enable */
    ULONG m_CounterConstantA[3];/* constant registers are write only copies are here */
    ULONG m_CounterConstantB[3];/* constant registers are write only copies are here */
    ULONG event_status;         /* interrupt event status */
    BYTE counter_num;           /* counter being serviced */
};


/*
    Defined below is the Interrupt Handler Data Structure.
    The interrupt handler is provided with a pointer that points
    to this structure.  From this the handler has a link back to
    its related process and common data area.  The usage of the members
    is not absolute.  Their typical uses are described below.
*/
struct handler_data
    {
    int h_pid;          /* Handler related process i.d. number.
                           Needed to know if handler is going to wake up,
                           put to sleep, or suspend a process.*/
    char *hd_ptr;       /* Handler related data pointer.
                           Needed to know if handler is going to access
                           a process' data area. */
    };



struct pmc464
{
    struct map464 *brd_ptr;		/* base address of the I/O board */
    struct dio464 *dio464ptr;           /* digital I/O section pointer */
    struct sdio464 *sdio464ptr;         /* Status block pointer */
    struct ct464 *ct464ptr;		/* counter/timer section pointer */
    int nHandle;                        /* handle to an open board */
    BOOL bPmc;                          /* flag indicating a board is open */
    BOOL bInitialized;			/* flag indicating ready to talk to board */
};


/*
    DECLARE MODULES CALLED:
*/


/* Declare digital I/O functions called */

void scfgdio(struct pmc464 *ptr);				/* routine to set up the configuration param. block */
void pstsdio(struct pmc464 *ptr);				/* routine which calls the Read Status Command */
void cnfgdio(struct pmc464 *c_blk);				/* routine to configure the digital I/O section of the board */
void rstsdio(struct pmc464 *c_blk);				/* routine to read status information */
long rpntdio(struct pmc464 *c_blk,
			unsigned port, unsigned point);		/* routine to read a input point */
long rprtdio(struct pmc464 *c_blk,
			unsigned port);				  /* routine to read the input port */
long wpntdio(struct pmc464 *c_blk,
			unsigned port, unsigned point, unsigned value);	     /* routine to write to a output point */
long wprtdio(struct pmc464 *c_blk,
			unsigned port, unsigned value);	/* routine to write to the output port */

/* ************************** End of Digital I/O section defines */



/* ************************** Start of Counter/Timer I/O section defines */

void trig(struct pmc464 *c_blk);			/* routine to software trigger */
void stop_trig(struct pmc464 *c_blk);		/* routine to software stop */
void setconf(struct pmc464 *c_blk);			/* routine to set up the configuration param. block */
void selectcnt(struct pmc464 *c_blk);		/* routine to examine/change the current counter */
void selectcon(struct pmc464 *c_blk);		/* routine to examine/change the counter constant*/
BOOL islongcounter(struct pmc464 *c_blk,
					int counter);			/* test for 32bit counter */
ULONG build_control(struct pmc464 *c_blk,
					int counter);			/* builds counter control words */


PMCSTAT ConfigureCounterTimer(struct pmc464 *c_blk,
					int counter);			/* Configure counter timer */
PMCSTAT ReadCounter(struct pmc464 *c_blk,
					int counter, ULONG *val);/* Read 16/32 bit counter */
PMCSTAT WriteCounterConstant(  struct pmc464 *c_blk,
					int counter);			/* Write counter constant to board register */
PMCSTAT StopCounter(struct pmc464 *c_blk,
					int counter);			/* Stop a counter */
PMCSTAT DisableInterrupt(struct pmc464 *c_blk,
					int counter);			/* Disable counter interrupts */
PMCSTAT GetDebounce(struct pmc464 *c_blk,
					int counter, word *debounce);/* get debounce */
PMCSTAT SetDebounce(struct pmc464 *c_blk,
					int counter, word debounce);/* get debounce */
PMCSTAT GetMode(struct pmc464 *c_blk,
					int counter, BYTE *mode);	/* get mode */
PMCSTAT SetMode(struct pmc464 *c_blk,
					int counter, BYTE mode);	/* set mode */
PMCSTAT GetInterruptEnable(struct pmc464 *c_blk,
					int counter, BOOL *enable);	/* get interrupt enable */
PMCSTAT SetInterruptEnable(struct pmc464 *c_blk,
					int counter, BOOL enable);	/* set interrupt enable */
PMCSTAT GetCounterSize(struct pmc464 *c_blk,
					int counter, BOOL *size);	/* get counter size 16/32 bit */
PMCSTAT SetCounterSize(struct pmc464 *c_blk,
					int counter, BOOL size);	/* set counter size 16/32 bit */
PMCSTAT GetClockSource(struct pmc464 *c_blk,
					int counter, BYTE *source);	/* get clock source */
PMCSTAT SetClockSource(struct pmc464 *c_blk,
					int counter, BYTE source);	/* set clock source */
PMCSTAT GetOutputPolarity(struct pmc464 *c_blk,
					int counter, BOOL *polarity);/* get output polarity */
PMCSTAT SetOutputPolarity(struct pmc464 *c_blk,
					int counter, BOOL polarity);/* set output polarity */
PMCSTAT GetInputAPolarity(struct pmc464 *c_blk,
					int counter, BYTE *polarity);/* get input A polarity */
PMCSTAT SetInputAPolarity(struct pmc464 *c_blk,
					int counter, BOOL polarity);/* set input A polarity */
PMCSTAT GetInputBPolarity(struct pmc464 *c_blk,
					int counter, BYTE *polarity);/* get input B polarity */
PMCSTAT SetInputBPolarity(struct pmc464 *c_blk,
					int counter, BOOL polarity);/* set input B polarity */
PMCSTAT GetInputCPolarity(struct pmc464 *c_blk,
					int counter, BYTE *polarity);/* get input C polarity */
PMCSTAT SetInputCPolarity(struct pmc464 *c_blk,
					int counter, BOOL polarity);/* set input C polarity */
PMCSTAT StartCounter(struct pmc464 *c_blk,
					int counter);				/* start counter */
PMCSTAT StartSimultaneousCounters(struct pmc464 *c_blk,
					word mask);					/* start simultaneous counters */
PMCSTAT StopSimultaneousCounters(struct pmc464 *c_blk,
					word mask);					/* stop simultaneous counters */
PMCSTAT GetCounterConstantA(struct pmc464 *c_blk,
					int counter, ULONG *val);	/* get constant */
PMCSTAT SetCounterConstantA(struct pmc464 *c_blk,
					int counter, ULONG val);	/* set constant */
PMCSTAT GetCounterConstantB(struct pmc464 *c_blk,
					int counter, ULONG *val);	/* get constant */
PMCSTAT SetCounterConstantB(struct pmc464 *c_blk,
					int counter, ULONG val);	/* set constant */
PMCSTAT ReadCounterControl(struct pmc464 *c_blk,
					int counter, ULONG *val);
PMCSTAT WriteCounterControl(struct pmc464 *c_blk,
					int counter, ULONG val);

long get_param(void);		/* input a parameter */

byte input_byte(int nHandle, byte*);/* function to read an input byte */
word input_word(int nHandle, word*);/* function to read an input word */
void output_byte(int nHandle, byte*, byte);	/* function to output a byte */
void output_word(int nHandle, word*, word);	/* function to output a word */
long input_long(int nHandle, long*);			/* function to read an input long */
void output_long(int nHandle, long*, long);	/* function to output a long */

/*
    declare interrupt handler
*/

void isr_pmc464(void* pAddr);	/* interrupt handler for Pmc464 */

#ifdef __cplusplus
}
#endif
