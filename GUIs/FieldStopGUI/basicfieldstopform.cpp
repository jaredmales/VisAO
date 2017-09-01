#include "basicfieldstopform.h"

#include <iostream>

namespace VisAO
{

BasicFieldStopForm::BasicFieldStopForm(QWidget * Parent, Qt::WindowFlags f) : basic_ui(Parent, f)
{

}

BasicFieldStopForm::BasicFieldStopForm(int argc, char **argv, QWidget * Parent, Qt::WindowFlags f) : basic_ui(argc, argv, Parent, f)
{
   Create();
}

BasicFieldStopForm::BasicFieldStopForm(std::string name, const std::string& conffile, QWidget * Parent, Qt::WindowFlags f) : basic_ui(name, conffile, Parent, f)
{
   Create();
}

#define HWP_FIFO_CH  0
#define FS_FIFO_CH  1

void BasicFieldStopForm::Create()
{
   ui.setupUi(this);
   attach_basic_ui();
   
   setup_fifo_list(2);
   
   std::string visao_root = getenv("VISAO_ROOT");
   std::string hwp_fifopath;
   std::string fs_fifopath;
   
   try
   {
      hwp_fifopath = (std::string)(ConfigDictionary())["hwp_fifopath"];
   }
   catch(...)
   {
      hwp_fifopath = "fifos";
   }
   std::string fpathin = visao_root + "/" + hwp_fifopath + "/hwprotator_com_local_in";
   std::string fpathout = visao_root + "/" + hwp_fifopath + "/hwprotator_com_local_out";

   
   signal(SIGIO, SIG_IGN);
   set_fifo_list_channel(&fl, HWP_FIFO_CH, 100,fpathout.c_str(), fpathin.c_str(), 0, 0);

   try
   {
      fs_fifopath = (std::string)(ConfigDictionary())["fs_fifopath"];
   }
   catch(...)
   {
      fs_fifopath = "fifos";
   }
   fpathin = visao_root + "/" + fs_fifopath + "/fieldstop_com_local_in";
   fpathout = visao_root + "/" + fs_fifopath + "/fieldstop_com_local_out";

   
   set_fifo_list_channel(&fl, FS_FIFO_CH, 100,fpathout.c_str(), fpathin.c_str(), 0, 0);
 

   
   if(connect_fifo_list() < 0)
   {
      QMessageBox msgBox;
      msgBox.setText("Another Field Stop GUI is probably already running.");
      msgBox.setInformativeText("Cannot lock the input FIFO on one or more channels.");
      msgBox.exec();
      exit(-1);
   }

   set_wait_to(2.);

   
   
   statustimerout();

   
   return;
}

void BasicFieldStopForm::attach_basic_ui()
{
   __LabelCmode = ui.LabelCmode;
   __StatusCmode = ui.StatusCmode;
   __StatusConnected = ui.StatusConnected;
   __ButtonTakeLocal = ui.ButtonTakeLocal;
   __checkBoxOverride = ui.checkBoxOverride;

   ui.cModeFrame->lower();
}

void BasicFieldStopForm::retrieve_state()
{
   retrieve_stateFS();
   
   //double startT;
   std::string resp, resFS;
   
   write_fifo_channel(HWP_FIFO_CH, "state?\n", 8, &resp);
   
   //startT = get_curr_time();
   if(resp == "" || resp.length()<10)
   {
      state_connected = 0;
      state_cmode = -1;      
      return;
   }
   
   state_connected = 1;
   
   
   state_cmode = resp[0];
   

   int st, sp;
   st = resp.find(',',0);
   sp = resp.find(',', st+1);

   if(st < 0 || sp < 0)
   {
      std::cerr << "Error parsing HWP state\n";
      state_connected = 0;
      state_cmode = -1;
      return;
   }
   hwpstate = atoi(resp.substr(st+1, sp-st-1).c_str());

   st = resp.find(',',sp);
   sp = resp.find(',', st+1);
   
   if(st < 0 || sp < 0)
   {
      std::cerr << "Error parsing power state\n";
      state_connected = 0;
      state_cmode = -1;
      return;
   }
   hwppower = atoi(resp.substr(st+1, sp-st-1).c_str());

   st = resp.find(',',sp);
   sp = resp.find(',', st+1);
   
   if(st < 0 || sp < 0)
   {
      std::cerr << "Error parsing moving\n";
      state_connected = 0;
      state_cmode = -1;
      return;
   }
   
   hwpmoving = atoi(resp.substr(st+1, sp-st-1).c_str());

   st = resp.find(',',sp);
   sp = resp.find(',', st+1);
   
   if(st < 0 || sp < 0)
   {
      std::cerr << "Error parsing homing\n";
      state_connected = 0;
      state_cmode = -1;
      return;
   }
   hwphoming = atoi(resp.substr(st+1, sp-st-1).c_str());

   
   st = resp.find(',',sp);
   //sp = resp.find(',', st+1);
   
   if(st < 0)
   {
      std::cerr << "Error parsing position\n";
      state_connected = 0;
      state_cmode = -1;
      return;
   }
   hwpposition = strtod(resp.substr(st+1, resp.length()-1).c_str(), 0);
   
   std::cout << "HWP: " << hwpposition << "\n";
   return;
   
}

void BasicFieldStopForm::retrieve_stateFS()
{
   //double startT;
   std::string resp;
   
   write_fifo_channel(FS_FIFO_CH, "state?\n", 8, &resp);
   
   //startT = get_curr_time();
   if(resp == "" || resp.length()<10)
   {
      state_connectedFS = 0;
      state_cmodeFS = -1;      
      return;
   }
   
   state_connectedFS = 1;
   
   
   state_cmodeFS = resp[0];
   
   int st, sp;
   st = resp.find(',',0);
   sp = resp.find(',', st+1);

   if(st < 0 || sp < 0)
   {
      std::cerr << "Error parsing FS state\n";
      state_connectedFS = 0;
      state_cmodeFS = -1;
      return;
   }
   fsstate = atoi(resp.substr(st+1, sp-st-1).c_str());
   
   st = resp.find(',',sp);
   sp = resp.find(',', st+1);
   
   if(st < 0 || sp < 0)
   {
      std::cerr << "Error parsing power state\n";
      state_connected = 0;
      state_cmode = -1;
      return;
   }
   fspower = atoi(resp.substr(st+1, sp-st-1).c_str());

   st = resp.find(',',sp);
   sp = resp.find(',', st+1);
   
   if(st < 0 || sp < 0)
   {
      std::cerr << "Error parsing moving\n";
      state_connected = 0;
      state_cmode = -1;
      return;
   }
   
   fsmoving = atoi(resp.substr(st+1, sp-st-1).c_str());

   st = resp.find(',',sp);
   sp = resp.find(',', st+1);
   
   if(st < 0 || sp < 0)
   {
      std::cerr << "Error parsing homing\n";
      state_connected = 0;
      state_cmode = -1;
      return;
   }
   fshoming = atoi(resp.substr(st+1, sp-st-1).c_str());

   
   st = resp.find(',',sp);
   //sp = resp.find(',', st+1);
   
   if(st < 0)
   {
      std::cerr << "Error parsing position\n";
      state_connected = 0;
      state_cmode = -1;
      return;
   }
   fsposition = strtod(resp.substr(st+1, resp.length()-1).c_str(), 0);
   
   std::cout << "Field Stop: " << fsposition << "\n";
   
   return;
   
}

void BasicFieldStopForm::update_statusFS()
{
   char str[256];
   
   if(state_connectedFS == 0)
   {
      ui.StatusConnected_FS->setStyleSheet("color: red");
      ui.StatusConnected_FS->setText("GUI not connected");
   }
   else
   {
      ui.StatusConnected_FS->setStyleSheet("color: gray");
      ui.StatusConnected_FS->setText("GUI connected");
   }

   switch(state_cmodeFS)
   {
      case 'N':
         ui.StatusCmode_FS->setText("NONE");
         break;
      case 'R':
         ui.StatusCmode_FS->setText("REMOTE");
         break;
      case 'L':
         ui.StatusCmode_FS->setText("LOCAL");
         break;
      case 'S':
         ui.StatusCmode_FS->setText("SCRIPT");
         break;
      case 'A':
         ui.StatusCmode_FS->setText("AUTO");
         break;
      default:
         ui.StatusCmode_FS->setText("UNK");
         break;
   }
   
   if(state_cmodeFS == 'L')
   {
      ui.ButtonTakeLocal_FS->setText("Give Up Local Control");
      ui.checkBoxOverride_FS->setChecked(false);
      ui.checkBoxOverride_FS->setEnabled(false);
   }
   else
   {
      ui.ButtonTakeLocal_FS->setText("Take Local Control");
      ui.checkBoxOverride_FS->setEnabled(true);
   }
   
   switch(fsstate)
   {
      case STATE_NOCONNECTION:
         ui.stateLabel_FS->setText("No Connection");
         break;
      case STATE_CONNECTED:
         ui.stateLabel_FS->setText("Connected");
         break;
      case STATE_HOMING:
         ui.stateLabel_FS->setText("Homing");
         break;
      case STATE_OPERATING:
         ui.stateLabel_FS->setText("Moving");
         break;
      case STATE_BUSY:
         ui.stateLabel_FS->setText("Continuous");
         break;
      case STATE_READY:
         ui.stateLabel_FS->setText("Ready");
         break;
      case STATE_OFF:
         ui.stateLabel_FS->setText("Power Off");
         break;
      case STATE_INVALID:
         ui.stateLabel_FS->setText("Invalid");
         break;
      case STATE_ERROR:
         ui.stateLabel_FS->setText("Error");
         break;
      default:
         ui.stateLabel_FS->setText("unknown state");
   }
   
   
   if(fsstate != STATE_NOCONNECTION && fsstate != STATE_INVALID && fsstate != STATE_ERROR && fsstate != STATE_OFF)
   {
      snprintf(str, 256, "%0.5f", fsposition);
      ui.FScurrPos->setText(str);

      
      if(state_cmodeFS == 'L')
      {
         ui.FSFieldStop->setEnabled(true);
         ui.FSHWP->setEnabled(true);
         ui.FSStow->setEnabled(true);
         ui.FSstopButton->setEnabled(true);
         ui.FSnewPos->setEnabled(true);
         ui.FSgoButton->setEnabled(true);
         ui.FShome->setEnabled(true);
      }
      else
      {
         ui.FSFieldStop->setEnabled(false);
         ui.FSHWP->setEnabled(false);
         ui.FSStow->setEnabled(false);
         ui.FSstopButton->setEnabled(false);
         ui.FSnewPos->setEnabled(false);
         ui.FSgoButton->setEnabled(false);
         ui.FShome->setEnabled(false);
         
      }
   }
   else
   {
      ui.FScurrPos->setText("?");
      ui.FSFieldStop->setEnabled(false);
      ui.FSHWP->setEnabled(false);
      ui.FSStow->setEnabled(false);
      ui.FSstopButton->setEnabled(false);
      ui.FSnewPos->setEnabled(false);
      ui.FSgoButton->setEnabled(false);
      ui.FShome->setEnabled(false);      
   }
   
   //update_status();
}

void BasicFieldStopForm::update_status()
{

   QString styleUp = "background-color : lime; color : black; ";
   QString styleDown = "background-color : red; color : black; ";
   QString styleCaution = "background-color : yellow; color : black; ";
   
   char str[256];
   
   switch(hwpstate)
   {
      case STATE_NOCONNECTION:
         ui.stateLabel->setText("No Connection");
         break;
      case STATE_CONNECTED:
         ui.stateLabel->setText("Connected");
         break;
      case STATE_HOMING:
         ui.stateLabel->setText("Homing");
         break;
      case STATE_OPERATING:
         ui.stateLabel->setText("Moving");
         break;
      case STATE_BUSY:
         ui.stateLabel->setText("Continuous");
         break;
      case STATE_READY:
         ui.stateLabel->setText("Ready");
         break;
      case STATE_OFF:
         ui.stateLabel->setText("Power Off");
         break;
      case STATE_INVALID:
         ui.stateLabel->setText("Invalid");
         break;
      case STATE_ERROR:
         ui.stateLabel->setText("Error");
         break;
      default:
         ui.stateLabel->setText("unknown state");
   }
   
   if(hwpstate != STATE_NOCONNECTION && hwpstate != STATE_INVALID && hwpstate != STATE_ERROR && hwpstate != STATE_OFF)
   {
      snprintf(str, 256, "%0.5f", hwpposition);
      ui.HWPcurrPos->setText(str);

      
      if(state_cmode == 'L')
      {
         ui.HWPminus45->setEnabled(true);
         ui.HWPminus225->setEnabled(true);
         ui.HWPstopButton->setEnabled(true);
         ui.HWPplus225->setEnabled(true);
         ui.HWPplus45->setEnabled(true);
         ui.HWPcontinuous->setEnabled(true);
         ui.HWPnewPos->setEnabled(true);
         ui.HWPgoButton->setEnabled(true);
         ui.HWPhome->setEnabled(true);
      }
      else
      {
         ui.HWPminus45->setEnabled(false);
         ui.HWPminus225->setEnabled(false);
         ui.HWPstopButton->setEnabled(false);
         ui.HWPplus225->setEnabled(false);
         ui.HWPplus45->setEnabled(false);
         ui.HWPcontinuous->setEnabled(false);
         ui.HWPnewPos->setEnabled(false);
         ui.HWPgoButton->setEnabled(false);
         ui.HWPhome->setEnabled(false);
      }
   }
   else
   {
      ui.HWPcurrPos->setText("?");
      ui.HWPminus45->setEnabled(false);
      ui.HWPminus225->setEnabled(false);
      ui.HWPstopButton->setEnabled(false);
      ui.HWPplus225->setEnabled(false);
      ui.HWPplus45->setEnabled(false);
      ui.HWPcontinuous->setEnabled(false);
      ui.HWPnewPos->setEnabled(false);
      ui.HWPgoButton->setEnabled(false);
      ui.HWPhome->setEnabled(false);
      
   }
   
   update_statusFS();
}

void BasicFieldStopForm::on_HWPstopButton_clicked()
{
   std::string resp;
   pthread_mutex_lock(&commutex);
   write_fifo_channel(HWP_FIFO_CH, "stop\n", 5, &resp);
   pthread_mutex_unlock(&commutex);
}
   
void BasicFieldStopForm::on_HWPminus45_clicked()
{
   std::string resp;
   pthread_mutex_lock(&commutex);
   write_fifo_channel(HWP_FIFO_CH, "m45\n", 4, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicFieldStopForm::on_HWPminus225_clicked()
{
   std::string resp;
   pthread_mutex_lock(&commutex);
   write_fifo_channel(HWP_FIFO_CH, "m225\n", 4, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicFieldStopForm::on_HWPplus225_clicked()
{
   std::string resp;
   pthread_mutex_lock(&commutex);
   write_fifo_channel(HWP_FIFO_CH, "p225\n", 4, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicFieldStopForm::on_HWPplus45_clicked()
{
   std::string resp;
   pthread_mutex_lock(&commutex);
   write_fifo_channel(HWP_FIFO_CH, "p45\n", 4, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicFieldStopForm::on_HWPcontinuous_clicked()
{
   std::string resp;
   pthread_mutex_lock(&commutex);
   write_fifo_channel(HWP_FIFO_CH, "controt\n", 8, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicFieldStopForm::on_HWPhome_clicked()
{
   std::string resp;
   pthread_mutex_lock(&commutex);
   write_fifo_channel(HWP_FIFO_CH, "home\n", 5, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicFieldStopForm::on_HWPgoButton_clicked()
{
   char str[256];
   std::string resp;
   double newx;

   newx = ui.HWPnewPos->text().toDouble();

   
   
   snprintf(str, 256, "pos %f", newx);
   
   pthread_mutex_lock(&commutex);
   write_fifo_channel(HWP_FIFO_CH, str, strlen(str)+1, &resp);
   pthread_mutex_unlock(&commutex);
}


void BasicFieldStopForm::on_ButtonTakeLocal_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   if(state_cmode == 'L')
   {
      write_fifo_channel(HWP_FIFO_CH, "~LOCAL\n", 8, &resp);
   }
   else
   {
      if(ui.checkBoxOverride->isChecked())
      {
         write_fifo_channel(HWP_FIFO_CH, "XLOCAL\n", 8, &resp);
      }
      else
      {
         write_fifo_channel(HWP_FIFO_CH, "LOCAL\n", 7, &resp);
      }
   }

   pthread_mutex_unlock(&commutex);
}




void BasicFieldStopForm::on_FSstopButton_clicked()
{
   std::string resp;
   pthread_mutex_lock(&commutex);
   write_fifo_channel(FS_FIFO_CH, "stop\n", 5, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicFieldStopForm::on_FSFieldStop_clicked()
{
   std::string resp;
   pthread_mutex_lock(&commutex);
   write_fifo_channel(FS_FIFO_CH, "gotoFlat\n", 9, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicFieldStopForm::on_FSHWP_clicked()
{
   std::string resp;
   pthread_mutex_lock(&commutex);
   write_fifo_channel(FS_FIFO_CH, "gotoHWP\n", 8, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicFieldStopForm::on_FSStow_clicked()
{
   std::string resp;
   pthread_mutex_lock(&commutex);
   write_fifo_channel(FS_FIFO_CH, "abs 0\n", 6, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicFieldStopForm::on_FSgoButton_clicked()
{
   char str[256];
   std::string resp;
   double newx;

   newx = ui.FSnewPos->text().toDouble();
   
   snprintf(str, 256, "abs %f", newx);
   
   pthread_mutex_lock(&commutex);
   write_fifo_channel(FS_FIFO_CH, str, strlen(str)+1, &resp);
   pthread_mutex_unlock(&commutex);
   
}

void BasicFieldStopForm::on_FShome_clicked()
{
   std::string resp;
   pthread_mutex_lock(&commutex);
   write_fifo_channel(FS_FIFO_CH, "home\n", 5, &resp);
   pthread_mutex_unlock(&commutex);
}   

void BasicFieldStopForm::on_ButtonTakeLocal_FS_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   if(state_cmodeFS == 'L')
   {
      write_fifo_channel(FS_FIFO_CH, "~LOCAL\n", 8, &resp);
   }
   else
   {
      if(ui.checkBoxOverride_FS->isChecked())
      {
         write_fifo_channel(FS_FIFO_CH, "XLOCAL\n", 8, &resp);
      }
      else
      {
         write_fifo_channel(FS_FIFO_CH, "LOCAL\n", 7, &resp);
      }
   }

   pthread_mutex_unlock(&commutex);
}

} //namespace VisAO
