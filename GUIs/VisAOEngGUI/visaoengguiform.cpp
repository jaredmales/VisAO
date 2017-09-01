#include "visaoengguiform.h"

namespace VisAO
{


   
VisAOEngGUIForm::VisAOEngGUIForm(QWidget * Parent, Qt::WindowFlags f) : QDialog(Parent, f)
{
   Create();
}

VisAOEngGUIForm::VisAOEngGUIForm(int argc, char **argv, QWidget * Parent, Qt::WindowFlags f) : QDialog(Parent, f),  VisAOApp_standalone(argc, argv)
{
   Create();
}

VisAOEngGUIForm::VisAOEngGUIForm(std::string name, const std::string& conffile, QWidget * Parent, Qt::WindowFlags f) : QDialog(Parent, f), VisAOApp_standalone(name, conffile)
{
   Create();
}

void VisAOEngGUIForm::Create()
{
   
   gimb_flashdelay = 0.5;
   gimb_flashstart = 0.;
   focus_flashdelay = 0.5;
   focus_flashstart = 0.;
         
   paramStyle = "background-color : lightgrey; color : black; ";
   upStyle = "background-color : lime; color : black; ";
   warnStyle = "background-color : yellow; color : black; ";
   softwarnStyle = "background-color : yellowgreen; color : black; ";
   downStyle = "background-color : red; color : black; ";
   normStyle = "";
   
   ui.setupUi(this);

   ui.ccd47GUI->set_conffile("ccd47ctrlGUI");
   ui.ccd47GUI->Create();

   ui.GimbalGUI->set_conffile("gimbalGUI");
   ui.GimbalGUI->Create();

   ui.FocusGUI->set_conffile("focusmotorGUI");
   ui.FocusGUI->Create();

   ui.filterWheel2->set_conffile("filterwheel2GUI");
   ui.filterWheel2->Create();

   ui.filterWheel3->set_conffile("filterwheel3GUI");
   ui.filterWheel3->Create();

   ui.bcu39GUI->set_conffile("bcu39GUI");
   ui.bcu39GUI->Create();
   
   ui.shutterGUI->set_conffile("shutterGUI");
   ui.shutterGUI->Create();

   ui.ccd47GUI->shutter = ui.shutterGUI;
   
   ui.wollastonGUI->set_conffile("wollastonstatusGUI");
   ui.wollastonGUI->Create();

//    ui.frameSelectorGUI->set_conffile("frameselectorGUI");
//    ui.frameSelectorGUI->Create();
//    
   ui.coronGuideGUI->set_conffile("coronguideGUI");
   ui.coronGuideGUI->Create();
   
   sysmonD = new  BasicsysmonDForm();
   sysmonD->set_conffile("sysmonDGUI");
   sysmonD->Create();
   
   tempmon = new  BasictempmonForm();
   tempmon->set_conffile("tempmonGUI");
   tempmon->Create();

   ccd47sb = 0;
   fg47sb = 0;
   fw47sb = 0;
   fs47sb = 0;
   fw2sb = 0;
   fw3sb = 0;
   wsb = 0;
   gsb = 0;
   diosb = 0;
   fsb = 0;
   ssb = 0;
   syssb = 0;
   fg39sb = 0;
   fw39sb = 0;
   recsb = 0;
   visaoisb = 0;
   psb = 0;
   fssb = 0;
   cgsb = 0;
   
   QString inyourfaceStyle = "background-color : black; color : lime; ";
   
   QString style = paramStyle;
   
   ui.ccd47exptime->setStyleSheet(style);
   ui.ccd47fps->setStyleSheet(style);
   ui.ccd47gain->setStyleSheet(style);
   ui.ccd47speed->setStyleSheet(style);
   ui.ccd47size->setStyleSheet(style);
   
   ui.gimbal->setStyleSheet(warnStyle);
   
   ui.bsName->setStyleSheet(paramStyle);
   ui.wollStat->setStyleSheet(paramStyle);
   ui.fw2Name->setStyleSheet(paramStyle);
   ui.fw3Name->setStyleSheet(paramStyle);
   ui.shutterState->setStyleSheet(paramStyle);
   
   statustimeout = 200;
   connect(&statustimer, SIGNAL(timeout()), this, SLOT(statustimerout()));

   statustimer.start(statustimeout);
   
   
   
   
   
   
}

void VisAOEngGUIForm::on_processesButton_clicked()
{
   system("$VISAO_ROOT/bin/sys_processes.py &");
}

void VisAOEngGUIForm::on_tempmonButton_clicked()
{
   tempmon->show();
}

void VisAOEngGUIForm::on_sysmonDButton_clicked()
{
   sysmonD->show();
}

// void VisAOEngGUIForm::on_gotoPreset_clicked()
// {
//    QMessageBox msgBox;
//    msgBox.setWindowModality(Qt::ApplicationModal);
//    msgBox.setWindowTitle("Go To Board Preset");
//    
//    msgBox.setIcon(QMessageBox::Question);
//    
//    QAbstractButton *yesButton = msgBox.addButton("Yes", QMessageBox::YesRole);
//    QAbstractButton *noButton = msgBox.addButton("No", QMessageBox::HelpRole);
//    
//    msgBox.setText("Go to VisAO board preset?");
//    
//    msgBox.exec();
//    
//    if(msgBox.clickedButton() == yesButton)
//    {
//       ui.FocusGUI->on_ButtonGOPreset_clicked();
//       ui.GimbalGUI->on_centerButton_clicked();
//       
//       return;
//    }
//    
//    return;
// }

// void VisAOEngGUIForm::on_savePreset_clicked()
// {
//    QMessageBox msgBox;
//    msgBox.setWindowModality(Qt::ApplicationModal);
//    msgBox.setWindowTitle("Save Board Preset");
//    
//    msgBox.setIcon(QMessageBox::Question);
// 
//    QAbstractButton *yesButton = msgBox.addButton("Yes", QMessageBox::YesRole);
//    QAbstractButton *noButton = msgBox.addButton("No", QMessageBox::HelpRole);
//       
//    msgBox.setText("Save VisAO Board Preset?");
// 
//    msgBox.exec();
// 
//    if(msgBox.clickedButton() == yesButton)
//    {
//       std::vector<double> fpst(1), gpst(2);
//       
//       //int save_preset(std::string calname, double fw1pos, int wollstat, double fw2pos, double fw3pos, std::vector<double> *vec);
// 
//       fpst[0] = fsb->cur_pos;
//       gpst[0] = gsb->xpos;
//       gpst[1] = gsb->ypos;
//       
//       ::save_preset("focus", visaoisb->filter1_pos, (int) wsb->cur_pos, fw2sb->pos, fw3sb->pos, &fpst);
//       ::save_preset("gimbal", visaoisb->filter1_pos, (int) wsb->cur_pos, fw2sb->pos, fw3sb->pos, &gpst);
//       
//       return;
//    }
// 
//    return;
// }

void VisAOEngGUIForm::statustimerout()
{
   char str[256];
   size_t sz;
   std::string gainstr;

   
   if(!ccd47sb) ccd47sb = (ccd47_status_board*) attach_shm(&sz,  STATUS_ccd47, 0);

   if(ccd47sb)
   {
      if((get_curr_time() - ts_to_curr_time(&ccd47sb->last_update)) < ccd47sb->max_update_interval)
      {
         ui.ccd47Status->setVisible(false);
         ui.ccd47Status->setStyleSheet(upStyle);
      
         std::string imtstr = "UNK";
         switch(ccd47sb->imtype)
         {
            case 0: imtstr = "science";
            break;
            case 1: imtstr = "acquis.";
            break;
            case 2: imtstr = "dark";
            break;
            case 3: imtstr = "sky";
            break;
            case 4: imtstr = "flat";
            break;
         }
         ui.ccd47imtype->setText(imtstr.c_str());
         
         //Update saving status
         if(ccd47sb->saving == 0)
         {
            ui.ccd47_saving->setText("-");
         }
         else
         {
            if(ccd47sb->remaining == -1)
            {
               //snprintf(str, 256, "%c", 236);
               ui.ccd47_saving->setText(trUtf8("\u221E"));
               
            }
            else
            {
               snprintf(str, 256, "%i", ccd47sb->remaining);
              ui.ccd47_saving->setText(str);
            }
         }
         //update config
         switch(ccd47sb->gain)
         {
            case 0:
               gainstr = "High";
               break;
            case 1:
               gainstr = "Med High";
               break;
            case 2:
               gainstr = "Med Low";
               break;
            case 3:
               gainstr = "Low";
               break;
            default:
               gainstr = "UNK";
         }
      
         snprintf(str, 256, "%0.3f sec", 1./ccd47sb->framerate);
         ui.ccd47exptime->setText(str);
         
         snprintf(str, 256, "%0.3f fps", ccd47sb->framerate);
         ui.ccd47fps->setText(str);
                
         ui.ccd47gain->setText(gainstr.c_str());
         
         snprintf(str, 256, "%i kHz", ccd47sb->speed);
         ui.ccd47speed->setText(str);
         
         snprintf(str, 256, "%i (%iX%i)", ccd47sb->windowx, ccd47sb->xbin, ccd47sb->ybin);
         ui.ccd47size->setText(str);
         
      
         

         
      }
      else
      {
         ui.ccd47Status->setVisible(true);
         ui.ccd47Status->setStyleSheet(downStyle);
         ui.ccd47_saving->setText("down");
        
         
      }   
   }
   else
   {
      ui.ccd47Status->setVisible(true);
      ui.ccd47Status->setStyleSheet(downStyle);
      ui.ccd47_saving->setText("down");
      
      
   }

   double ct = get_curr_time();
   
   if(!fg47sb) fg47sb = (basic_status_board*) attach_shm(&sz,  STATUS_framegrabber47, 0);
   check_watchdog(fg47sb, ui.fg47Status, ct);
   
   if(!fw47sb) fw47sb = (basic_status_board*) attach_shm(&sz,  STATUS_framewriter47, 0);
   check_watchdog(fw47sb, ui.fw47Status, ct);
   
   if(!fs47sb) fs47sb = (basic_status_board*) attach_shm(&sz,  STATUS_frameserver47, 0);
   check_watchdog(fs47sb, ui.fs47Status, ct);

   if(!fw2sb) fw2sb = (filterwheel_status_board*) attach_shm(&sz,  STATUS_filterwheel2, 0);
   check_watchdog(fw2sb, ui.fw2Status, ct);
   if(fw2sb)
   {
      ui.fw2Name->setText(fw2sb->filter_name);
   }
   else
   {
      ui.fw2Name->setText("?");
   }
   
   if(!fw3sb) fw3sb = (filterwheel_status_board*) attach_shm(&sz,  STATUS_filterwheel3, 0);
   check_watchdog(fw3sb, ui.fw3Status, ct);
   if(fw3sb)
   {
      ui.fw3Name->setText(fw3sb->filter_name);
   }
   else
   {
      ui.fw3Name->setText("?");
   }
   
   if(!wsb) wsb = (wollaston_status_board*) attach_shm(&sz,  STATUS_wollaston, 0);
   check_watchdog(wsb, ui.wollStatus, ct);
   if(wsb)
   {
      if(wsb->cur_pos == 1)  ui.wollStat->setText("IN");
      else if(wsb->cur_pos ==-1) ui.wollStat->setText("OUT");
      else ui.wollStat->setText("intermediate");
   }
   else ui.wollStat->setText("?");

   if(wsb && fw2sb && fw3sb)
   {
      bool wollwarn = false;
      bool combowarn = false;
      bool dwwarn = false;
      
      if(wsb->cur_pos == 1 && fw3sb->type != 1) wollwarn = true;
      if(wsb->cur_pos == -1 && fw3sb->type == 1) dwwarn = true;
      if(wsb->cur_pos != 1 && wsb->cur_pos != -1) wollwarn = true;
      
      if(strcmp(fw2sb->filter_name, "open") == 0 && fw3sb->type != 1) combowarn = true;
      if(strcmp(fw2sb->filter_name, "open") != 0 && fw3sb->type == 1) combowarn = true;
         
      if(strcmp(fw2sb->filter_name, "intermediate") == 0 ) combowarn = false;
      if(strcmp(fw3sb->filter_name, "intermediate") == 0) wollwarn = false;
      
      if(dwwarn)
      {
         ui.fw3Name->setStyleSheet(softwarnStyle);
         ui.wollStat->setStyleSheet(softwarnStyle);
      }
      else
      {
         ui.fw3Name->setStyleSheet(paramStyle);
         ui.wollStat->setStyleSheet(paramStyle);
      }
      
      if(wollwarn || combowarn) ui.fw3Name->setStyleSheet(warnStyle);
      else if(!dwwarn) ui.fw3Name->setStyleSheet(paramStyle);
   
      if(combowarn) ui.fw2Name->setStyleSheet(warnStyle);
      else ui.fw2Name->setStyleSheet(paramStyle);
      
      if(wollwarn) ui.wollStat->setStyleSheet(warnStyle);
      else if(!dwwarn) ui.wollStat->setStyleSheet(paramStyle);
   }
   
   if(!gsb) gsb = (gimbal_status_board*) attach_shm(&sz,  STATUS_gimbal, 0);
   check_watchdog(gsb, ui.gimbalStatus, ct);
   
   if(!cgsb) cgsb = (coronguide_status_board*) attach_shm(&sz, STATUS_coronguide, 0);
   check_watchdog(cgsb, ui.cguideStatus, ct);
   
   if(gsb && !cgsb)
   {
      
      if(gsb->xpos != gimb_lastx || gsb->ypos != gimb_lasty)
      {
         ui.gimbal->setStyleSheet(warnStyle);
         ui.gimbal->setVisible(true);
         gimb_flashstart = get_curr_time();
      }
      else if (gimb_flashstart > 0)
      {
         if(get_curr_time()-gimb_flashstart > gimb_flashdelay)
         {
            gimb_flashstart = 0;
            ui.gimbal->setVisible(false);
         }
      }
      gimb_lastx = gsb->xpos;
      gimb_lasty = gsb->ypos;

   }
   else if (gsb && cgsb)
   {
      if(cgsb->loopState == 0)
      {
         if(gsb->xpos != gimb_lastx || gsb->ypos != gimb_lasty)
         {
            ui.gimbal->setStyleSheet(warnStyle);
            ui.gimbal->setText("Gimbal");
            ui.gimbal->setVisible(true);
            gimb_flashstart = get_curr_time();
         }
         else if (gimb_flashstart > 0)
         {
            if(get_curr_time()-gimb_flashstart > gimb_flashdelay)
            {
               gimb_flashstart = 0;
               ui.gimbal->setVisible(false);
            }
         }
         else
         {
            ui.gimbal->setVisible(false);
            ui.gimbal->setStyleSheet(warnStyle);
            ui.gimbal->setText("Gimbal");
         }
         gimb_lastx = gsb->xpos;
         gimb_lasty = gsb->ypos;
      }
      else
      {
         gimb_flashstart=0;
         ui.gimbal->setStyleSheet(softwarnStyle);
         if(cgsb->loopState == 1) ui.gimbal->setText("CG CLOSED");
         if(cgsb->loopState == 2) ui.gimbal->setText("CG PAUSED");
         if(cgsb->loopState == 3) ui.gimbal->setText("AUTOPAUSE");
         if(cgsb->loopState == 4) ui.gimbal->setText("AP RECOVERY");
         ui.gimbal->setVisible(true);
      }
   }  
   else ui.gimbal->setVisible(false);
   
   if(!diosb) diosb = (basic_status_board*) attach_shm(&sz,  STATUS_dioserver, 0);
   check_watchdog(diosb, ui.dioStatus, ct);

   if(!fsb) fsb = (focusstage_status_board*) attach_shm(&sz,  STATUS_focusstage, 0);
   check_watchdog(fsb, ui.focStatus, ct);
   if(fsb)
   {
      if(fsb->is_moving)
      {
         ui.focusState->setText("focus stage");
         ui.focusState->setStyleSheet(warnStyle);
         focus_flashstart = get_curr_time();
      }
      else if(focus_flashstart > 0)
      {
         if(get_curr_time()-focus_flashstart > focus_flashdelay)
         {
            focus_flashstart = 0;
            ui.focusState->setStyleSheet("");
         }
      }
      
      if(focus_flashstart == 0)
      {
         if(fsb->is_focused) 
         {
            ui.focusState->setStyleSheet("");
            ui.focusState->setText("in focus");
         }
         else
         {
            ui.focusState->setText("NOT FOCUSED");
            ui.focusState->setStyleSheet(downStyle);
         }
      }
   }
      
   if(!ssb) ssb = (shutter_status_board*) attach_shm(&sz,  STATUS_shutterctrl, 0);
   check_watchdog(ssb, ui.shutStatus, ct);

   if(!fssb) fssb = (frameselector_status_board*) attach_shm(&sz,  STATUS_frameselector, 0);
   check_watchdog(fssb, ui.rtfsStatus, ct);
   
   if(ssb)
   {
      if(ssb->in_auto)
      {
         if(fssb)
         {
            if(fssb->frame_select)
            {
               ui.shutterState->setText("RTFS");
               ui.shutterState->setStyleSheet(softwarnStyle);
            }
            else
            {
               ui.shutterState->setText("auto");
               ui.shutterState->setStyleSheet(paramStyle);
            }
         }
         else 
         {
            ui.shutterState->setText("auto");
            ui.shutterState->setStyleSheet(paramStyle);
         }
      }
      else
      {
         ui.shutterState->setStyleSheet(paramStyle);
         if(ssb->sw_state == 1) ui.shutterState->setText("open");
         if(ssb->sw_state == 0) ui.shutterState->setText("unkown");
         if(ssb->sw_state == -1) ui.shutterState->setText("shut");
      }
   }
   
   //Check if shutter state and imtype are consistent
   if(ccd47sb && ssb)
   {
      if(!ssb->in_auto)
      {
         bool darkwarn = false;
         bool shutwarn = false;
      
         if(ssb->sw_state == -1 && ccd47sb->imtype != 2) darkwarn = true;
         if(ssb->sw_state == 1 && ccd47sb->imtype == 2) darkwarn = true;
         if(ssb->sw_state != 1 && ssb->sw_state != -1) shutwarn = true;
               
         if(darkwarn) ui.ccd47imtype->setStyleSheet(warnStyle);
         else ui.ccd47imtype->setStyleSheet("");
      
         if(shutwarn || darkwarn) ui.shutterState->setStyleSheet(warnStyle);
         else ui.shutterState->setStyleSheet(paramStyle);
      }
      else ui.ccd47imtype->setStyleSheet("");
   }
   
   
   if(!syssb) syssb = (system_status_board*) attach_shm(&sz,  STATUS_sysmonD, 0);
   check_watchdog(syssb, ui.sysmonStatus, ct);
   if(syssb)
   {
      int syswarn = 0;
               
      //check disk full status here
      
      double hddperc = ((double)syssb->dfroot_used)/((double)syssb->dfroot_size);
      
      if(hddperc > syssb->hdd_used_warn) syswarn = 1;
      if(hddperc > syssb->hdd_used_limit) syswarn = 2;
      
      if(syssb->raid_stat[0] !='U' || syssb->raid_stat[1] !='U' || syssb->raid_stat[4] !='U' || syssb->raid_stat[5] != 'U' )
      {
         syswarn = 2;
      }

      if(syswarn == 0) ui.sysmonDButton->setStyleSheet("");
      if(syswarn == 1) ui.sysmonDButton->setStyleSheet(warnStyle);
      if(syswarn == 2) ui.sysmonDButton->setStyleSheet(downStyle);
         

      int tempwarn = 0;
      
      for(int i=0;i<SYS_N_CORES; i++) if(syssb->core_temps[i] >= syssb->core_temp_warn) tempwarn = 1;
      if(syssb->GPUTemp >= syssb->gpu_temp_warn) tempwarn = 1;
      if(syssb->HDDTemp_a >= syssb->hdd_temp_warn) tempwarn = 1;
      if(syssb->HDDTemp_b >= syssb->hdd_temp_warn) tempwarn = 1;
      if(syssb->AirTemp >= syssb->air_temp_warn) tempwarn = 1;
      if(ccd47sb) if(ccd47sb->joe_temp >= syssb->joe_temp_warn) tempwarn = 1;
      
      for(int i=0;i<SYS_N_CORES; i++) if(syssb->core_temps[i] >= syssb->core_temp_limit) tempwarn = 2;
      if(syssb->GPUTemp >= syssb->gpu_temp_limit) tempwarn = 2;
      if(syssb->HDDTemp_a >= syssb->hdd_temp_limit) tempwarn = 2;
      if(syssb->HDDTemp_b >= syssb->hdd_temp_limit) tempwarn = 2;
      if(syssb->AirTemp >= syssb->air_temp_limit) tempwarn = 2;
      if(ccd47sb) if(ccd47sb->joe_temp >= syssb->joe_temp_limit) tempwarn = 2;
      
      if(tempwarn == 0) ui.tempmonButton->setStyleSheet("");
      if(tempwarn == 1) ui.tempmonButton->setStyleSheet(warnStyle);
      if(tempwarn == 2) ui.tempmonButton->setStyleSheet(downStyle);
      
   }
   
   
   if(!fg39sb) fg39sb = (framegrabber39_status_board*) attach_shm(&sz,  STATUS_framegrabber39, 0);
   check_watchdog(fg39sb, ui.fg39Status, ct);

   if(!fw39sb) fw39sb = (framegrabber39_status_board*) attach_shm(&sz,  STATUS_framewriter39, 0);
   check_watchdog(fw39sb, ui.fw39Status, ct);

   if(!recsb) recsb = (framegrabber39_status_board*) attach_shm(&sz,  STATUS_reconstructor, 0);
   check_watchdog(recsb, ui.recStatus, ct);

   if(!visaoisb) visaoisb = (aosystem_status_board*) attach_shm(&sz,  STATUS_aosystem, 0);
   check_watchdog(visaoisb, ui.visaoiStatus, ct);

   if(visaoisb)
   {
      ui.bsName->setText(visaoisb->filter1_name);
      if(visaoisb->loop_on == 0) ui.loopStat->setText("loop open");
      if(visaoisb->loop_on == 1) ui.loopStat->setText("loop closed");
      if(visaoisb->loop_on == 2) ui.loopStat->setText("loop paused");
      
      snprintf(str, 256, "%i Hz", (int) (visaoisb->ccd39_freq + 0.5));
      ui.loopFreq->setText(str);
      
      snprintf(str, 256, "%i x %i", visaoisb->ccd39_bin, visaoisb->ccd39_bin);
      ui.loopBin->setText(str);
      
      snprintf(str, 256, "%i", visaoisb->correctedmodes);
      ui.loopModes->setText(str);
      
      ui.target->setText(visaoisb->catobj);
      
      double ha = visaoisb->ha;
    
      int sgn = 1;
      if(ha < 0) sgn =-1;
      snprintf(str, 256, "%-02i:%02i:%02i", (int) ha, sgn*((int) ((ha - ((int) ha))*60.)), sgn*((int) (((ha - (int) ha) - ((int) ((ha - ((int) ha))*60.))/60.)*3600.))  );
      ui.tgtHA->setText(str);
      
      snprintf(str, 256, "%0.2f", visaoisb->am);
      ui.tgtAM->setText(str);
        
   }
   else
   {
      ui.bsName->setText("?");
   }
   
   if(!psb) psb = (power_status_board*) attach_shm(&sz,  STATUS_power, 0);
   check_watchdog(psb, ui.pwrmonStatus, ct);
   
}

void VisAOEngGUIForm::check_watchdog(basic_status_board *bsb, QLabel * ql, double ct)
{
   if(ct == 0)
   {
      ct = get_curr_time();
   }
   
   if(bsb)
   {
      if((ct - ts_to_curr_time(&bsb->last_update)) < 2.*bsb->max_update_interval)
      {
         ql->setVisible(false);
         ql->setStyleSheet(upStyle);
      }
      else 
      {
         ql->setVisible(true);
         if((ct - ts_to_curr_time(&bsb->last_update)) < 5.*bsb->max_update_interval)
         {
            ql->setStyleSheet(warnStyle);
         }
         else ql->setStyleSheet(downStyle);
      }      
   }
   else
   {
      ql->setVisible(true);
      ql->setStyleSheet(downStyle);
   }
}


} //namespace VisAO

