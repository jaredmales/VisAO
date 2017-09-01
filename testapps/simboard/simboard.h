/************************************************************
*    simboard.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for a class to simulate the VisAO Status Boards.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file simboard.h
  * \author Jared R. Males
  * \brief Declarations for a class to simulate the VisAO Status Boards.
  * 
  *
*/

#ifndef __simboard_h__
#define __simboard_h__

#include "VisAOApp_standalone.h"

namespace VisAO
{

class simboard :  public VisAOApp_standalone
{
public:
   simboard(int argc, char **argv) throw (AOException);
   simboard(std::string name, const std::string &conffile) throw (AOException);

protected:
   
   ///Creates the status boards and processes the config file
   void Create();
      
   focusstage_status_board * fsb;

   ccd47_status_board * ccd47sb;
   
   shutter_status_board * shsb;
   
   filterwheel_status_board * fw2sb;
   
   filterwheel_status_board * fw3sb;
   
   system_status_board * syssb;
   
   wollaston_status_board *wsb;
   
   gimbal_status_board * gsb;

public:
   
   virtual int Run();
   
};


} //namespace VisAO


#endif //__simboard_h__
