/************************************************************
*    PwrMon.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for the PwrMon class, which monitors the VisAOPwrCtrl
* application.  VisAO power control is managed from the supervisor computer
* in the interest of reliability, but the VisAO system needs access to it.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file PwrMon.h
  * \author Jared R. Males
  * \brief Declarations for the VisAOPwrMon class
  *
  * VisAOPwrMon monitors the VisAOPwrCtrl
  * application.  VisAO power control is managed from the supervisor computer
  * in the interest of reliability, but the VisAO system needs access to it.
  *
*/

#ifndef __PwrMon_h__
#define __PwrMon_h__


#include "AOApp.h"
#include "VisAOApp_base.h"



namespace VisAO
{

///A class to monitor the VisAOPwrCtrl via the RTDB.
class PwrMon : public AOApp, public VisAOApp_base
{
public:
   ///Standard adopt style config file constructor.
   PwrMon( std::string name, const std::string &conffile) throw (AOException);

   ///Standard adopt style command line constructor.
   PwrMon( int argc, char **argv) throw (AOException);

protected:
   
   ///Basic setup of the class, called by both constructors.
   void setupVisAOApp();
   
   /// Virtual function to setup variables in RTDB
   void SetupVars();

   RTDBvar var_outlet1_cur, var_outlet1_req, var_outlet1_name;
   RTDBvar var_outlet2_cur, var_outlet2_req, var_outlet2_name;
   RTDBvar var_outlet3_cur, var_outlet3_req, var_outlet3_name;
   RTDBvar var_outlet4_cur, var_outlet4_req, var_outlet4_name;
   RTDBvar var_outlet5_cur, var_outlet5_req, var_outlet5_name;
   RTDBvar var_outlet6_cur, var_outlet6_req, var_outlet6_name;
   RTDBvar var_outlet7_cur, var_outlet7_req, var_outlet7_name;
   RTDBvar var_outlet8_cur, var_outlet8_req, var_outlet8_name;
public:
   static int outlet1CurChanged(void *pt, Variable *msgb);
   static int outlet2CurChanged(void *pt, Variable *msgb);
   static int outlet3CurChanged(void *pt, Variable *msgb);
   static int outlet4CurChanged(void *pt, Variable *msgb);
   static int outlet5CurChanged(void *pt, Variable *msgb);
   static int outlet6CurChanged(void *pt, Variable *msgb);
   static int outlet7CurChanged(void *pt, Variable *msgb);
   static int outlet8CurChanged(void *pt, Variable *msgb);

protected:
   virtual void Run();

   virtual int init_statusboard();

   /// Update the status board.
   /** Calls this as VisAOApp_bas::update_statusboard so the basics are taken care of.
    * \retval 0 on success
    * \retval -1 on failure
    */
   virtual int update_statusboard();


};

} //namespace VisAO

#endif //__PwrMon_h__

