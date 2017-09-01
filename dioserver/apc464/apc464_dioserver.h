
#ifndef __APC464_DIOSERVER_H__
#define __APC464_DIOSERVER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pmccommon/pmcmulticommon.h"
#include "pmc464.h"


typedef struct
{
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
} apc464_info;

int init_apc464_info(apc464_info * apc464);

int apc464_init(void * diocard_info);

int apc464_write(void * diocard_info, int ch, int xput);

int apc464_read(void * diocard_info, int ch);


#ifdef __cplusplus
} //extern "C"
#endif

#endif // __APC464_DIOSERVER_H__
