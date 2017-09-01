
#ifndef __VisAOIClient_h__
#define __VisAOIClient_h__


#include "MagAO/MagAOIClient/MagAOIClient.h"
#include "VisAOApp_base.h"

#define WOLL_FIFO  0
#define FOC_FIFO   1
#define GIMB_FIFO  2

//#define _debug

namespace VisAO
{

class VisAOIClient : public MagAOIClient, public VisAOApp_base
{
   public:
      VisAOIClient(int argc, char **argv) throw(AOException);
      VisAOIClient( std::string name, const std::string &conffile) throw (AOException);
   protected:
      void Create(void) throw (AOException);

      std::string fifo_path; ///<The path to the fifos, relative to VISAO_ROOT

      VisAO::aosystem_status_board * aosb;
            
      /** @name Wollaston Remote Control
        * VisAOIClient is the interface for "control" of the Wollaston
        * from the supervisor.
        */
      //@{
      std::string wollaston_process;
      RTDBvar wollaston_status;
      RTDBvar wollaston_cmode_cur;
      RTDBvar wollaston_cmode_req;

      RTDBvar wollaston_state_cur;
      RTDBvar wollaston_state_req;

      static int WollStateReqChanged(void *pt, Variable *msgb);
      static int WollCModeReqChanged(void *pt, Variable *msgb);

      int send_wollaston_command(std::string com);
      int update_wollaston();
      //@}

      /** @name Focus Remote Control
        * VisAOIClient is the interface for "control" of the Focus Stage
        * from the supervisor.
        */
      //@{
      std::string focus_process;
      RTDBvar focus_cmode_cur;
      RTDBvar focus_cmode_req;

      RTDBvar focus_pos_cur;
      RTDBvar focus_pos_req;
      RTDBvar focus_abort_req;

      RTDBvar focus_preset_req;
      
      RTDBvar focus_status;
      RTDBvar focus_limsw_cur;

      static int FocusPosReqChanged(void *pt, Variable *msgb);
      static int FocusAbortReqChanged(void *pt, Variable *msgb);
      static int FocusCModeReqChanged(void *pt, Variable *msgb);
      static int FocusPresetReqChanged(void *pt, Variable *msgb);
      
      int send_focus_command(std::string com);
      int update_focus();
      //@}

      /** @name Gimbal Remote Control
       * VisAOIClient is the interface for control of the Gimbal
       * from the supervisor.
       */
      //@{
      std::string gimbal_process;
      RTDBvar gimbal_status;
      RTDBvar gimbal_xpos_cur;
      RTDBvar gimbal_ypos_cur;
      RTDBvar gimbal_cmode_cur;
      RTDBvar gimbal_cmode_req;
         
      RTDBvar gimbal_center_req;
      RTDBvar gimbal_dark_req;
        
      static int GimbalCModeReqChanged(void *pt, Variable *msgb);
      static int GimbalCenterReqChanged(void *pt, Variable *msgb);
      static int GimbalDarkReqChanged(void *pt, Variable *msgb);

      int send_gimbal_command(std::string com);
      int update_gimbal();
      //@}

      /** @name Master Remote Control
       * VisAOIClient provides a master control over VisAO for supervisor
       */
      //@{
      RTDBvar var_masterremote_req;
      RTDBvar ccd47_cmode_req;
      RTDBvar shutter_cmode_req;
      RTDBvar fw2_cmode_req;
      RTDBvar fw3_cmode_req;
      
      ///When notified, VisAOIClient changes the control mode of the entire VisAO Camera.
      static int MasterRemoteReqChanged(void *pt, Variable *msgb);

      ///Change control of Gimbal, Focus, F/W2, F/W3, CCD47.
      int setMasterRemote(int cmode);
      
      //@}
         
      ///Load the configuration details from the file
      int LoadConfig();

      virtual void SetupVars();
      
      // VIRTUAL - Run
      virtual void Run();

      virtual void post_update_DD_var(DD_RTDBVar &var);

      void update_loopon();
      
      void init_statusboard();
      int  update_statusboard();
      void dump_statusboard();

      VisAO::reconstructor_status_board *rsb;
      void update_recon();
      
      RTDBvar var_avgwfe;
      RTDBvar var_stdwfe;
      RTDBvar var_instwfe;
      
      int orient_useel;
      int orient_usepa;
  
     

   public:
      ///Log data at intervals
      virtual void dataLogger(timeval tv);
      
};




} //namespace VisAO

#endif //__VisAOIClient_h__
