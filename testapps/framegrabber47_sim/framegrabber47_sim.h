/************************************************************
*    framegrabber47_sim.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for a class to manage a simulated framegrabber of CCD47 data.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file framegrabber47_sim.h
  * \author Jared R. Males
  * \brief Declarations for a class to manage a simulated framegrabber  of CCD47 data.
  * 
  *
*/

#ifndef __framegrabber47_sim__
#define __framegrabber47_sim__

#include "framegrabber.h"


namespace VisAO
{

class framegrabber47_sim : public framegrabber<short>
{
public:
   framegrabber47_sim(int argc, char **argv) throw (AOException);
   framegrabber47_sim(std::string name, const std::string &conffile) throw (AOException);
   
protected:
   void init_framegrabber47_sim();
   
   int sim_mode;
   double sim_rate;
   
   short **im1024;
   short **im128;
   
public:
   virtual int start_framegrabber();
   virtual int stop_framegrabber();
   
};

int get_fits_im(short *im, long hduno, long *fpix, long *lpix, fitsfile **fptr, const char *fname);

int get_fits_im(double *im, long hduno, long *fpix, long *lpix, fitsfile **fptr, char *fname);

int load_128x128(short ** imd128);

} //namespace VisAO


#endif //__framegrabber47_sim_h__
