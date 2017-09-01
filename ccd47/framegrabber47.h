/************************************************************
*    framegrabber47.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for a class to manage the EDT PCI framegrabber for the CCD47.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file framegrabber47.h
  * \author Jared R. Males
  * \brief Declarations for a class to manage the EDT PCI framegrabber for the CCD47.
  * 
  *
*/

#ifndef __framegrabber47_h__
#define __framegrabber47_h__

#include "framegrabber.h"

#include "edtinc.h"

namespace VisAO
{

///The framegrabber specialization for the CCD47.
/** This class provides the CCD47 framegrabber which writes to a sharedim_stack.
 * Manages the EDT PCI DVI framegrabber card.
 *
 * In addition to the \ref framegrabber config file options, has:
 *  - <b>num_bufs</b> <tt>int</tt> [4] - the number of DMA buffers allocated by the EDT driver
 *
 * See \ref VisAOApp_standalone for command line arguments.  There are no additional command line arguments for
 * a basic framegrabber, though derived clases may add them.
 *
 * See \ref framegrabber for the commands accepted by this class.
 *
 */
class framegrabber47 : public framegrabber<short>
{
   public:
      framegrabber47(int argc, char **argv) throw (AOException);
      framegrabber47(std::string name, const std::string &conffile) throw (AOException);

   protected:
      void init_framegrabber47();
            
      char    edt_devname[128];//= '\0';
      int     channel;// = 0;
      int     unit;// = 0;
      PdvDev *pdv_p;
      
      int     num_bufs;// = 4;
      
      char   *cameratype;
      
      int     overrun;
      int     overruns;//=0;
      int     timeouts; 
      int     last_timeouts;// = 0;
      
      int     recovering_timeout;// = FALSE;
          
      int frameCtr;
      
   public:
      virtual int start_framegrabber();
      virtual int stop_framegrabber();
      
};

} //namespace VisAO

#endif //__framegrabber47_h__
