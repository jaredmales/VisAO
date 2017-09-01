#include "frameselectorform.h"

//extern fifo_list * global_fifo_list;


#define FS_FIFO_CH 0

namespace VisAO
{
   
FrameSelectorForm::FrameSelectorForm(QWidget * Parent, Qt::WindowFlags f) : basic_ui(Parent, f)
{
   
}

FrameSelectorForm::FrameSelectorForm(int argc, char **argv, QWidget * Parent, Qt::WindowFlags f) : basic_ui(argc, argv, Parent, f)
{
   Create();
}

FrameSelectorForm::FrameSelectorForm(std::string name, const std::string &conffile, QWidget * Parent, Qt::WindowFlags f) : basic_ui(name, conffile, Parent, f)
{
   Create();
}

void FrameSelectorForm::Create()
{
   ui.setupUi(this);
   attach_basic_ui();

   
   signal(SIGIO, SIG_IGN);
   
   setup_fifo_list(1);
   
   std::string visao_root = getenv("VISAO_ROOT");
   std::string stfifo = (std::string)(ConfigDictionary())["fifo_path"];
      
   std::string fpathin = visao_root + "/" + stfifo + "/frameselector_com_local_in";
   std::string fpathout = visao_root + "/" + stfifo + "/frameselector_com_local_out";
   set_fifo_list_channel(&fl, FS_FIFO_CH, 100,fpathout.c_str(), fpathin.c_str(), 0, 0);
   
  
   
   if(connect_fifo_list() < 0)
   {
      QMessageBox msgBox;
      msgBox.setText("Another frameselector GUI is probably already running.");
      msgBox.setInformativeText("Cannot lock the input FIFO on one or more channels.");
      msgBox.exec();
      exit(-1);
   }
   
      
   
   
   
   ui.labelRTFSState->setStyleSheet(paramStyle);
   ui.labelCurrThresh->setStyleSheet(paramStyle);
   
   return;
}

void FrameSelectorForm::attach_basic_ui()
{
   __LabelCmode = ui.LabelCmode;
   __StatusCmode = ui.StatusCmode;
   __StatusConnected = ui.StatusConnected;
   __ButtonTakeLocal = ui.ButtonTakeLocal;
   __checkBoxOverride = ui.checkBoxOverride;

   ui.cModeFrame->lower();
}

void FrameSelectorForm::retrieve_state()
{
   std::string resp;
   
   write_fifo_channel(FS_FIFO_CH, "state?\n", 6, &resp);
   
   
   if(resp == "" || resp.length() < 5)
   {
      return not_connected();
   }
   state_cmode = resp[0];
   
   int idx, edx;
   
   idx = resp.find_first_not_of(" ", 1);
   edx = resp.find_first_of(" ", idx+1);      
   if(idx < 0 || edx < 0 || edx < idx) return not_connected();
   curr_state = atoi(resp.substr(idx, edx-idx).c_str());
   
   
   curr_thresh = strtod(resp.substr(edx+1, resp.length()-edx-1).c_str(), 0);
   
            
   state_connected = 1;
   return;
}

void FrameSelectorForm::not_connected()
{
   state_connected = 0;
   
   curr_state = 0;
   curr_thresh = 0;
   
   return;
}

void FrameSelectorForm::update_status()
{
   char tmp[50];  
   
   if(curr_state == 1 && state_connected)
   {
      ui.labelRTFSState->setText("selecting");
      ui.rtfsStart->setText("Stop Selecting");
   }
   
   if(curr_state == 0 && state_connected)
   {
      ui.labelRTFSState->setText("idle");
      ui.rtfsStart->setText("Start Selecting");
   }
   
   if(state_connected)
   {
      snprintf(tmp, 50, "%0.3f", curr_thresh);
      ui.labelCurrThresh->setText(tmp);
   }
   
   if(!state_connected)
   {
      ui.labelRTFSState->setText("?");
      ui.labelCurrThresh->setText("?");  
   }
   
   if(state_cmode != 'L')
   {
      ui.rtfsStart->setEnabled(false);
      ui.newThreshButton->setEnabled(false);
      ui.labelNewThresh->setEnabled(false);
      ui.inputNewThresh->setEnabled(false);
   }
   else
   {
      ui.rtfsStart->setEnabled(true);
      if(curr_state == 1)
      {
         ui.newThreshButton->setEnabled(false);
         ui.labelNewThresh->setEnabled(false);
         ui.inputNewThresh->setEnabled(false);
      }
      else
      {
         ui.newThreshButton->setEnabled(true);
         ui.labelNewThresh->setEnabled(true);
         ui.inputNewThresh->setEnabled(true);
      }
   }
            
                                   
}



void FrameSelectorForm::on_rtfsStart_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   
   if(curr_state == 1)
   {
      write_fifo_channel(FS_FIFO_CH, "stop", 4, &resp);
   }
   if(curr_state == 0)
   {
      write_fifo_channel(FS_FIFO_CH, "start", 5, &resp);
   }

   pthread_mutex_unlock(&commutex);
   
   statustimerout();
}


void FrameSelectorForm::on_newThreshButton_clicked()
{
   std::string resp;
   char tmp[256];
   
   pthread_mutex_lock(&commutex);

   snprintf(tmp, 256, "thresh %f", ui.inputNewThresh->text().toDouble());
   write_fifo_channel(FS_FIFO_CH, tmp, strlen(tmp), &resp);
   
   pthread_mutex_unlock(&commutex);
   
}
void FrameSelectorForm::on_ButtonTakeLocal_clicked()
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
