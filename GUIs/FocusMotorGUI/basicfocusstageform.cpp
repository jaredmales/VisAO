#include "basicfocusstageform.h"

namespace VisAO
{

BasicFocusStageForm::BasicFocusStageForm(QWidget * Parent, Qt::WindowFlags f) : basic_ui(Parent,f)
{
}

BasicFocusStageForm::BasicFocusStageForm(int argc, char **argv, QWidget * Parent, Qt::WindowFlags f) : basic_ui(argc, argv, Parent, f)
{
   Create();
}

BasicFocusStageForm::BasicFocusStageForm(std::string name, const std::string& conffile, QWidget * Parent, Qt::WindowFlags f) : basic_ui(name, conffile, Parent, f)
{
   Create();
}

void BasicFocusStageForm::Create()
{
   ui.setupUi(this);	
   attach_basic_ui();

   ui.toolBox->setCurrentIndex(0);

   QString style = "background-color : lightgrey; color : black; ";
   ui.currentPosition->setStyleSheet(style);
   ui.labelPositionUnits->setStyleSheet(style);


   offset = 0.0;
   
   ui.lineEditPosition->setFocusPolicy(Qt::ClickFocus);
   setup_fifo_list(1);

   std::string visao_root = getenv("VISAO_ROOT");
   std::string FocusMotor_fifopath = (std::string)(ConfigDictionary())["FocusMotor_fifopath"];
   
   std::string fpathin = visao_root + "/" + FocusMotor_fifopath + "/focusmotor_com_local_in";
   std::string fpathout = visao_root + "/" + FocusMotor_fifopath + "/focusmotor_com_local_out";

   signal(SIGIO, SIG_IGN);
   set_fifo_list_channel(&fl, 0, 100,fpathout.c_str(), fpathin.c_str(), 0, 0);

   
   //global_fifo_list = &fl;

   if(connect_fifo_list() < 0)
   {
      //pthread_mutex_lock(&commutex);
      QMessageBox msgBox;
      msgBox.setText("Another Focus GUI is probably already running.");
      msgBox.setInformativeText("Cannot lock the input FIFO on one or more channels.");
      msgBox.exec();
      exit(-1);
   }

   wait_to = .2;
   //write_fifo_channel(0, "LOCAL\n", 6);
   statustimerout();

   return;
}

void BasicFocusStageForm::attach_basic_ui()
{
   __LabelCmode = ui.LabelCmode;
   __StatusCmode = ui.StatusCmode;
   __StatusConnected = ui.StatusConnected;
   __ButtonTakeLocal = ui.ButtonTakeLocal;
   __checkBoxOverride = ui.checkBoxOverride;

   ui.cModeFrame->lower();
}

void BasicFocusStageForm::retrieve_state()
{
   std::string resp;
   
   write_fifo_channel(0, "state?\n", 8, &resp);
   
   if(resp == "")
   {
      state_connected = 0;
      state_cmode = -1;
      state_pos = -1e12;
      state_is_moving = -2;
      state_remaining = -1e-12;
      state_power = 0;
      return;
   }
   
   state_connected = 1;
   
   //std::cout << resp << "\n";
   if(resp.length() > 15)
   {
      state_cmode = resp[0];
      
      state_pos = strtod(resp.substr(2,12).c_str(),0);
      //std::cout << state_pos << "\n";
      
      state_enabled = atoi(resp.substr(15,1).c_str());
      
      state_power = atoi(resp.substr(17,1).c_str());
      
      state_is_moving = atoi(resp.substr(19,1).c_str());
      
      state_homing = atoi(resp.substr(21,1).c_str());
      
      state_neg_limit = atoi(resp.substr(23,1).c_str());
      state_home_switch = atoi(resp.substr(25,1).c_str());
      state_pos_limit = atoi(resp.substr(27,1).c_str());
      
      state_remaining = strtod(resp.substr(29,12).c_str(),0);
      
      state_pos_limit_disabled = atoi(resp.substr(42,1).c_str());
      
      //std::cout << resp << "\n";
      //std::cout << (char) state_cmode << " " << state_pos << " " << state_is_moving << " " << state_remaining << "\n";
   }
   return;

}

void BasicFocusStageForm::update_status()
{
   char posstr[12];
   snprintf(posstr, 12, "%0.4f", state_pos);

   if(state_connected)
   {
      ui.currentPosition->setText(posstr);
      ui.labelPositionUnits->setText("um");
   

      if(state_power) ui.powerState->setText("ON");
      else ui.powerState->setText("OFF");
   
      if(state_enabled)
      {
         ui.statusEnable->setText("Enabled");
         ui.buttonEnable->setText("Disable");
      }
      else
      {
         ui.statusEnable->setText("Disabled");
         ui.buttonEnable->setText("Enable");
      }
   
      if(state_neg_limit)
      {
         ui.checkFront->setChecked(true);
         ui.statusOther->setText("Negative Limit");
      }
      else ui.checkFront->setChecked(false);
   
      if(state_home_switch)
      {
         ui.checkHome->setChecked(true);
         ui.statusOther->setText("Home Switch");
      }
      else ui.checkHome->setChecked(false);
   
      if(state_pos_limit)
      {
         ui.checkBack->setChecked(true);
         ui.statusOther->setText("Positive Limit");
      }
      else ui.checkBack->setChecked(false);

      if(state_pos_limit_disabled)
      {
         QString style = "color : red; ";
         ui.statusOther->setStyleSheet(style);
         ui.statusOther->setText("POSITIVE LIMIT DISABLED!");
      }
      else
      {
         //QString style = ;
         ui.statusOther->setStyleSheet("color : black; ");
         if(!state_neg_limit && !state_home_switch && !state_pos_limit)  ui.statusOther->setText("");
      }
   }
   if(state_connected == 0 || state_cmode != 'L')
   {
      ui.ButtonGO->setEnabled(false);
      ui.ButtonGOPreset->setEnabled(false);
 
      ui.ButtonPos->setEnabled(false);
      ui.ButtonNeg->setEnabled(false);
      ui.ButtonAbort->setEnabled(true);
      ui.buttonEnable->setEnabled(false);
      ui.ButtonHome->setEnabled(false);
      ui.ButtonHomeNeg->setEnabled(false);
      ui.ButtonHomePos->setEnabled(false);
      if(!state_connected)
      {
         ui.powerState->setText("");
         ui.currentPosition->setText("");
         ui.labelPositionUnits->setText("");
         ui.statusOther->setText("");
         ui.statusEnable->setText("");
         ui.checkBack->setChecked(false);
         ui.checkFront->setChecked(false);
         ui.checkHome->setChecked(false);
      }
      return;
   }
   
   if(state_is_moving && !state_homing)
   {
      ui.lineEditPosition->clearFocus();
      ui.ButtonGO->setEnabled(false);
      ui.ButtonGOPreset->setEnabled(false);
      
      ui.ButtonPos->setEnabled(false);
      ui.ButtonNeg->setEnabled(false);
      ui.ButtonAbort->setEnabled(true);
      ui.buttonEnable->setEnabled(false);
      ui.ButtonHome->setEnabled(false);
      ui.ButtonHomeNeg->setEnabled(false);
      ui.ButtonHomePos->setEnabled(false);
      
      snprintf(posstr, 12, "%0.4f", state_remaining);
      ui.statusOther->setText(posstr);
      return;
   }

   if(state_homing)
   {
      ui.lineEditPosition->clearFocus();
      ui.ButtonGO->setEnabled(false);
      ui.ButtonGOPreset->setEnabled(false);
      
      ui.ButtonPos->setEnabled(false);
      ui.ButtonNeg->setEnabled(false);
      ui.ButtonAbort->setEnabled(true);
      ui.buttonEnable->setEnabled(false);
      ui.ButtonHome->setEnabled(false);
      ui.ButtonHomeNeg->setEnabled(false);
      ui.ButtonHomePos->setEnabled(false);
      
      switch(state_homing)
      {
         case 1:
            ui.statusOther->setText("Initial Homing");
            break;
         case 2:
            ui.statusOther->setText("Main Homing");
            break;
         case 3:
            ui.statusOther->setText("Secondary Homing");
            break;
         case 4:
            ui.statusOther->setText("Negative Homing");
            break;
         case 5:
            ui.statusOther->setText("Positive Homing");
            break;
      }
      return;
   }

   ui.ButtonGO->setEnabled(true);
   ui.ButtonGOPreset->setEnabled(true);
   
   ui.ButtonPos->setEnabled(true);
   ui.ButtonNeg->setEnabled(true);
   ui.ButtonAbort->setEnabled(true);
   ui.buttonEnable->setEnabled(true);
   ui.ButtonHome->setEnabled(true);
   ui.ButtonHomeNeg->setEnabled(true);
   ui.ButtonHomePos->setEnabled(true);
 
   

}

void BasicFocusStageForm::on_ButtonGO_clicked()
{
   std::string resp;
   char posstr[20];
   if(!state_is_moving && state_cmode == 'L')
   {
      double newpos = ui.lineEditPosition->text().toDouble();
      ui.lineEditPosition->clearFocus();
      
      snprintf(posstr, 20, "pos %0.4f\n", newpos);

      pthread_mutex_lock(&commutex);
      write_fifo_channel(0, posstr, strlen(posstr), &resp);
      pthread_mutex_unlock(&commutex);
   }
}

void BasicFocusStageForm::on_ButtonGOPreset_clicked()
{
   std::string resp;

   if(!state_is_moving && state_cmode == 'L')
   {
      ui.lineEditPosition->clearFocus();
      
      pthread_mutex_lock(&commutex);
      write_fifo_channel(0, "preset\n", 8, &resp);
      pthread_mutex_unlock(&commutex);
   }
}

   
void BasicFocusStageForm::on_ButtonPos_clicked()
{
   std::string resp;
   //std::cout << "raise\n";
   char offstr[20];
   snprintf(offstr,20,"dpos %0.4f\n", fabs(offset));
   
   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, offstr, strlen(offstr), &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicFocusStageForm::on_ButtonNeg_clicked()
{
   std::string resp;
   char offstr[20];
   snprintf(offstr,20,"dpos %0.4f\n", -1*fabs(offset));
   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, offstr, strlen(offstr), &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicFocusStageForm::on_ButtonAbort_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, "abort\n", 7, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicFocusStageForm::on_ButtonHome_clicked()
{
   std::string resp;
   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, "home\n", 6, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicFocusStageForm::on_ButtonHomeNeg_clicked()
{
   std::string resp;
   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, "homeneg\n", 9, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicFocusStageForm::on_ButtonHomePos_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, "homepos\n", 9, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicFocusStageForm::on_buttonEnable_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   if(state_enabled) write_fifo_channel(0, "disable\n", 6, &resp);
   else write_fifo_channel(0, "enable\n", 6, &resp);

   pthread_mutex_unlock(&commutex);
}

void BasicFocusStageForm::on_lineEditOffset_editingFinished()
{
   offset = ui.lineEditOffset->text().toDouble();
}

void BasicFocusStageForm::on_ButtonTakeLocal_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   
   if(state_cmode == 'L')
   {
      write_fifo_channel(0, "~LOCAL\n", 8, &resp);
   }
   else
   {
      if(ui.checkBoxOverride->isChecked())
      {
         write_fifo_channel(0, "XLOCAL\n", 8, &resp);
      }
      else
      {
         write_fifo_channel(0, "LOCAL\n", 7, &resp);
      }
   }

   pthread_mutex_unlock(&commutex);
}

} //namespace VisAO

