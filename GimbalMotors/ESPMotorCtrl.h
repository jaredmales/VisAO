/************************************************************
 *    ESPMotorCtrl.h
 *
 * Author: Jared R. Males (jrmales@email.arizona.edu)
 *
 * Declarations for a Newport ESP motor controller.
 *
 * Developed as part of the Magellan Adaptive Optics system.
 ************************************************************/

/** \file ESPMotorCtrl.h
 * \author Jared R. Males
 * \brief Declarations for a Newport ESP motor controller.
 *
 */

#ifndef __ESPMotorCtrl_h__
#define __ESPMotorCtrl_h__

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cmath>

///The namespace of VisAO software
namespace VisAO
{

#define MOVE_RESOLUTION  0.00005

/// The ESP motor controller class.
/** Manges a set of ESP motor controllers chained together.
 */
class ESPMotorCtrl 
{
public:
   /// Default constructor.
   ESPMotorCtrl();

   /// Specify number of axes.
   ESPMotorCtrl(int no);

   /// Destructor
   virtual ~ESPMotorCtrl();

protected:

   std::stringstream errStr;
   
   int numAxes; ///< Number of motor controllers

   std::vector<int> axisAddress; ///< The controller address of each axis
   
   std::vector<std::string> stageName; ///< The names of the connected motors

   void getStageName(int axis, std::string &sname);

   int default_timeout;

public:
   ///Get the error string
   std::string getErrString(){return errStr.str();}
   
   int setNumAxes(int no); ///<Sets the number of axes.  Can only be called when uninitialized

   int powerOnInit(); ///< Checks the controllers for current errors, and populates the stageName vector

   char getError(int axis);
   
   std::string getStageName(int axis);

   int getCtrlState(int axis, std::string &state);
   
   double getCurPos(int axis);

   int home(int axis);

   int stop(int axis = 0);
   
   int gotoAbsPos(int axis, double apos);

   int gotoRelPos(int axis, double rpos);

   
   int makeCom(std::string &str, int axis, const char *com);
   int makeCom(std::string &str, int axis, const char *com, int val);
   int makeCom(std::string &str, int axis, const char *com, double val);
   int makeCom(std::string &str, int axis, const char *com, std::string &val);

   int splitResponse(int &axis, std::string &com, std::string &val, std::string &resp);
   
   virtual int sendCommand(std::string &com, std::string &resp, int timeout=1000);

   virtual int sendCommand(std::string &com);
   
   ///Called before starting a move.  Empty here, to be overridden.
   virtual void moveStart();

};

} //namespace VisAO

#endif //__ESPMotorCtrl_h__
