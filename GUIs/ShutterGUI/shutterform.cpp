#include "shutterform.h"

//extern fifo_list * global_fifo_list;

#define SH_FIFO_CH  0
#define FS_FIFO_CH 1

namespace VisAO
{
   
ShutterForm::ShutterForm(QWidget * Parent, Qt::WindowFlags f) : basic_ui(Parent, f)
{
   
}

ShutterForm::ShutterForm(int argc, char **argv, QWidget * Parent, Qt::WindowFlags f) : basic_ui(argc, argv, Parent, f)
{
   Create();
}

ShutterForm::ShutterForm(std::string name, const std::string &conffile, QWidget * Parent, Qt::WindowFlags f) : basic_ui(name, conffile, Parent, f)
{
   Create();
}

void ShutterForm::Create()
{
   ui.setupUi(this);
   attach_basic_ui();

   
   signal(SIGIO, SIG_IGN);
   
   setup_fifo_list(1);
   
   std::string visao_root = getenv("VISAO_ROOT");
   std::string stfifo = (std::string)(ConfigDictionary())["fifo_path"];
      
   std::string fpathin = visao_root + "/" + stfifo + "/shuttercontrol_com_local_in";
   std::string fpathout = visao_root + "/" + stfifo + "/shuttercontrol_com_local_out";
   set_fifo_list_channel(&fl, SH_FIFO_CH, 100,fpathout.c_str(), fpathin.c_str(), 0, 0);
   
   
   
   if(connect_fifo_list() < 0)
   {
      QMessageBox msgBox;
      msgBox.setText("Another shutter GUI is probably already running.");
      msgBox.setInformativeText("Cannot lock the input FIFO on one or more channels.");
      msgBox.exec();
      exit(-1);
   }
   
      
   statustimeout_normal = statustimeout;
   
   ui.labelShutterState->setStyleSheet(paramStyle);
   
   
   return;
}

void ShutterForm::open()
{
   std::string resp;
   
   pthread_mutex_lock(&commutex);
   
   write_fifo_channel(SH_FIFO_CH, "open", 4, &resp);
   
   pthread_mutex_unlock(&commutex);
}

void ShutterForm::shut()
{
   std::string resp;
   
   pthread_mutex_lock(&commutex);

   write_fifo_channel(SH_FIFO_CH, "close", 5, &resp);

   pthread_mutex_unlock(&commutex);
}

void ShutterForm::attach_basic_ui()
{
   __LabelCmode = ui.LabelCmode;
   __StatusCmode = ui.StatusCmode;
   __StatusConnected = ui.StatusConnected;
   __ButtonTakeLocal = ui.ButtonTakeLocal;
   __checkBoxOverride = ui.checkBoxOverride;

   ui.cModeFrame->lower();
}

void ShutterForm::retrieve_state()
{
   std::string resp;
   
   write_fifo_channel(SH_FIFO_CH, "state?\n", 6, &resp);
   
   if(resp == "")
   {
      return not_connected();
   }
   state_cmode = resp[0];
   
   if(state_cmode == 'A')
   {
      state_connected = 1;
      curr_state = 2;
      sw_state = 2;
      hw_state = 2;
      pwr_state = 2;
      return;
   }
   
   int idx, edx;
   
   idx = resp.find_first_not_of(" ", 1);
   edx = resp.find_first_of(" ", idx+1);      
   if(idx < 0 || edx < 0 || edx < idx) return not_connected();
   curr_state = atoi(resp.substr(idx, edx-idx).c_str());
   
   idx = resp.find_first_not_of(" ", edx);
   edx = resp.find_first_of(" ", idx+1);
   if(idx < 0 || edx < 0 || edx < idx) return not_connected();      
   sw_state = atoi(resp.substr(idx, edx-idx).c_str());
   
   idx = resp.find_first_not_of(" ", edx);
   edx = resp.find_first_of(" ", idx+1);  
   if(idx < 0 || edx < 0 || edx < idx) return not_connected();    
   hw_state = atoi(resp.substr(idx, edx-idx).c_str());
   
   idx = resp.find_first_not_of(" ", edx);
   edx = resp.find_first_of(" ", idx+1);      
   if(idx < 0 ) return not_connected();
   pwr_state = atoi(resp.substr(idx, edx-idx).c_str()); 
            
   state_connected = 1;
   return;
}

void ShutterForm::not_connected()
{
   state_connected = 0;
   
   curr_state = 0;
   
   return;
}

void ShutterForm::update_status()
{
     
   
   if(curr_state == 1 && pwr_state == 1)
   {
      ui.labelShutterState->setText("OPEN");
      ui.pushButtonOpenShut->setText("Shut");
   }
   
   if(curr_state == -1 && pwr_state == 1)
   {
      ui.labelShutterState->setText("SHUT");
      ui.pushButtonOpenShut->setText("Open");
   }
   
   if(curr_state != 2 && (curr_state == 0 || pwr_state != 1))
   {
      ui.labelShutterState->setText("?");
      ui.pushButtonOpenShut->setText("?");
   }
   
   if(curr_state == 2)
   {
      ui.labelShutterState->setText("AUTO");
      ui.pushButtonOpenShut->setText("---");
   }   

   if(pwr_state == 1)
   {
      ui.powerState->setText("on");
   }
   else if(pwr_state == 2)
   {
      ui.powerState->setText("on?");
   }
   else
   {
      ui.powerState->setText("off");     
   }
   
   if(state_cmode != 'L')
   {
      ui.pushButtonOpenShut->setEnabled(false);
   }  
   else
   {
      
      if(pwr_state == 1)
      {
         ui.pushButtonOpenShut->setEnabled(true);
      }
      else
      {
         ui.pushButtonOpenShut->setEnabled(false);
      }
   }                              
}



void ShutterForm::on_pushButtonOpenShut_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   
   if(curr_state == 1)
   {
      write_fifo_channel(SH_FIFO_CH, "close", 5, &resp);
   }
   if(curr_state == -1)
   {
      write_fifo_channel(SH_FIFO_CH, "open", 4, &resp);
   }

   pthread_mutex_unlock(&commutex);
   
   statustimerout();
}




void ShutterForm::on_ButtonTakeLocal_clicked()
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
