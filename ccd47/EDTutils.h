/************************************************************
*    EDTutils.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for some utility functions pertaining to the EDT framegrabber PCI card.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file EDTutils.h
  * \author Jared R. Males
  * \brief Declarations for some utility functions pertaining to the EDT framegrabber PCI card.
  * 
  *
*/


#ifndef __EDTutils_h__
#define __EDTutils_h__

#ifdef __cplusplus
extern "C"
{
#endif

#include "edtinc.h"

#include "edt_trace.h"

#include <stdlib.h>
#include <ctype.h>

///A local version of initcam.
/** Just a copy of initcam.c, but with only fname for options
  */ 
int initcam(const char * fname, const char *bdir);

///A local version of setdebug
/** Just a copy of setdebug.c, but with only the -k option
  */
int setdebug(const char * kbuff);

#ifdef __cplusplus
} //extern "C"
#endif

#endif //__EDTutils_h__
