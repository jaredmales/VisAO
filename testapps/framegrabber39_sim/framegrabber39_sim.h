/************************************************************
*    framegrabber39_sim.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for a class to manage a simulated framegrabber of CCD39 data.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file framegrabber_sim.h
  * \author Jared R. Males
  * \brief Declarations for a class to manage a simulated framegrabber of CCD39 data.
  * 
  *
*/

#ifndef __framegrabber39_sim__
#define __framegrabber39_sim__

#include "framegrabber.h"

#include "bcu_diag.h"

namespace VisAO
{

///A simulated framegrabber for the BCU-39
class framegrabber39_sim : public framegrabber<unsigned char>
{
public:
   framegrabber39_sim(int argc, char **argv) throw (AOException);
   framegrabber39_sim(std::string name, const std::string &conffile) throw (AOException);
   ~framegrabber39_sim();
   
protected:
   void init_framegrabber39_sim();

   void load_caos_data();
   void load_bcu39_data();
   
   int sim_mode;
   
   double sim_rate;

   unsigned char **im39;
   
   short **impyr;
   
   float **slopes;

   sharedim_stackD slopes_sis; ///< The shared memory ring buffer for slopes storage
   sharedimD * slopes_sim; ///< Pointer to a shared memory image of slopes
   
   double loopt0;
   double frameCnt;
public:
   //virtual int Run();
   
   virtual int start_framegrabber();
   virtual int stop_framegrabber();

   virtual int update_statusboard();
   
};

/*int get_fits_im(unsigned char *im, long hduno, long *fpix, long *lpix, fitsfile **fptr, const char *fname);

int get_fits_im(short *im, long hduno, long *fpix, long *lpix, fitsfile **fptr, const char *fname);

int get_fits_im(double *im, long hduno, long *fpix, long *lpix, fitsfile **fptr, const char *fname);*/


} //namespace VisAO


#endif //__framegrabber_sim_h__
