/************************************************************
*    shutter_tester.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Class to manage testing of the VisAO shutter.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file shutter_tester.h
  * \author Jared R. Males (jrmales@email.arizona.edu)
  * \brief Class to manage testing of the VisAO shutter.
  *
*/

#ifndef __SHUTTER_TESTER_H
#define __SHUTTER_TESTER_H

#include "libvisao.h"
#include "VisAOApp_standalone.h"
#include "readcolumns.h"

#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <string>

namespace VisAO
{

class shutter_tester : public VisAOApp_standalone
{
public:
   shutter_tester(int argc, char **argv) throw (AOException);
   shutter_tester(std::string name, const std::string &conffile) throw (AOException);
   
   int seq_no;
   
protected:
   std::string shutter_fifo;
   
   int initialize_shutter_tester() throw (AOException);
   
   int testmode; ///<What type of test to run
   enum testmodes{testfreq, testsim, testmode_max};
   
   std::string testfile; ///<File to open to drive the test
   double simDeltaT; ///<The delta-time per strehl entry in the test file
   
   int act_state; ///< The actual (current) state of the shutter 1=open, -1=shut
   
   double threshold; ///<When using a simulated strehl curve, the threshold
   
   double freq; ///<For the frequency mode.  In Hz.
   double max_freq; ///< The maximum allowable frequency.
   
   double dutycyc; ///< The ratio of time open to time shut.
   
   double duration; ///<Duration of the test.
   
   double t_remaining; ///<Time remaining in test.
   
   int RunTest;
   
public:
   std::string local_command(std::string com);
   
   int set_testmode(int sm);
   int get_testmode(){return testmode;}
   
   int set_testfile(std::string tf);
   std::string get_testfile(){return testfile;}
   
   int set_threshold(double st);
   double get_threshold(){return threshold;}
   
   int set_freq(double f);
   double get_freq(){return freq;}
   
   int set_dutycyc(double dc);
   double get_dutycyc(){return dutycyc;}
   
   int set_duration(double d);
   double get_duration(){return duration;}
   
   
   int Run(); ///<Start the tester.
   int run_test();///<Run the test.
   
   ///Test the shutter using the specified frequency and duty cycle.
   int freq_test();
   
   ///Test the shutter using a simulation file.
   int sim_test();
   
   virtual int update_statusboard();

};

} //namespace VisAO

#endif // __SHUTTER_TESTER_H

