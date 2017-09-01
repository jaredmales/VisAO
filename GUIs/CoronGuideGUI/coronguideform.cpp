#include "coronguideform.h"

//extern fifo_list * global_fifo_list;

#define CG_FIFO_CH  0


namespace VisAO
{
   
CoronGuideForm::CoronGuideForm(QWidget * Parent, Qt::WindowFlags f) : basic_ui(Parent, f)
{
   
}

CoronGuideForm::CoronGuideForm(int argc, char **argv, QWidget * Parent, Qt::WindowFlags f) : basic_ui(argc, argv, Parent, f)
{
   Create();
}

CoronGuideForm::CoronGuideForm(std::string name, const std::string &conffile, QWidget * Parent, Qt::WindowFlags f) : basic_ui(name, conffile, Parent, f)
{
   Create();
}

void CoronGuideForm::Create()
{
   ui.setupUi(this);
   attach_basic_ui();

   
   signal(SIGIO, SIG_IGN);
   
   setup_fifo_list(1);
   
   std::string visao_root = getenv("VISAO_ROOT");
   std::string stfifo = (std::string)(ConfigDictionary())["fifo_path"];
      
   std::string fpathin = visao_root + "/" + stfifo + "/coronguide_com_local_in";
   std::string fpathout = visao_root + "/" + stfifo + "/coronguide_com_local_out";
   set_fifo_list_channel(&fl, CG_FIFO_CH, 100,fpathout.c_str(), fpathin.c_str(), 0, 0);

   if(connect_fifo_list() < 0)
   {
      QMessageBox msgBox;
      msgBox.setText("Another coronguide GUI is probably already running.");
      msgBox.setInformativeText("Cannot lock the input FIFO on one or more channels.");
      msgBox.exec();
      exit(-1);
   }

   ui.labelLoopState->setStyleSheet(paramStyle);
   ui.labelLoopGain->setStyleSheet(paramStyle);
   ui.labelBGAlgo->setStyleSheet(paramStyle);
   ui.labelAvgCurr->setStyleSheet(paramStyle);

   ui.bgAlgoCombo->insertItem(0, "");
   ui.bgAlgoCombo->insertItem(1, "Median Sub");
   ui.bgAlgoCombo->insertItem(2, "Unsharp Mask");
  
   //ui.kfwhmCurr->setStyleSheet(paramStyle);
   
   return;
}

void CoronGuideForm::attach_basic_ui()
{
   __LabelCmode = ui.LabelCmode;
   __StatusCmode = ui.StatusCmode;
   __StatusConnected = ui.StatusConnected;
   __ButtonTakeLocal = ui.ButtonTakeLocal;
   __checkBoxOverride = ui.checkBoxOverride;

   ui.cModeFrame->lower();
}

void CoronGuideForm::retrieve_state()
{
   std::string resp;
   
   write_fifo_channel(CG_FIFO_CH, "state?\n", 6, &resp);
   
   if(resp == "")
   {
      return not_connected();
   }
   state_cmode = resp[0];
   
   
   int idx, edx;
   
   idx = resp.find_first_not_of(" ", 1);
   edx = resp.find_first_of(" ", idx+1);      
   if(idx < 0 || edx < 0 || edx < idx) return not_connected();
   loop_state = atoi(resp.substr(idx, edx-idx).c_str());
   
   idx = resp.find_first_not_of(" ", edx);
   edx = resp.find_first_of(" ", idx+1);
   if(idx < 0 || edx < 0 || edx < idx) return not_connected();      
   loop_gain = strtod(resp.substr(idx, edx-idx).c_str(),0);
   
   idx = resp.find_first_not_of(" ", edx);
   edx = resp.find_first_of(" ", idx+1);  
   if(idx < 0 || edx < 0 || edx < idx) return not_connected();    
   tgtx = strtod(resp.substr(idx, edx-idx).c_str(),0);
   
   idx = resp.find_first_not_of(" ", edx);
   edx = resp.find_first_of(" ", idx+1);      
   if(idx < 0 || edx < 0 || edx < idx ) return not_connected();
   tgty = strtod(resp.substr(idx, edx-idx).c_str(),0); 
            
   idx = resp.find_first_not_of(" ", edx);
   edx = resp.find_first_of(" ", idx+1);      
   if(idx < 0 || edx < 0 || edx < idx) return not_connected();
   xcen = strtod(resp.substr(idx, edx-idx).c_str(),0);
   
   idx = resp.find_first_not_of(" ", edx);
   edx = resp.find_first_of(" ", idx+1);
   if(idx < 0 || edx < 0 || edx < idx) return not_connected();      
   ycen = strtod(resp.substr(idx, edx-idx).c_str(),0);
   
   idx = resp.find_first_not_of(" ", edx);
   edx = resp.find_first_of(" ", idx+1);
   if(idx < 0 || edx < 0 || edx < idx) return not_connected();      
   bgAlgorithm = atoi(resp.substr(idx, edx-idx).c_str());
   
   idx = resp.find_first_not_of(" ", edx);
   edx = resp.find_first_of(" ", idx+1);
   if(idx < 0 || edx < 0 || edx < idx) return not_connected();      
   kfwhm = strtod(resp.substr(idx, edx-idx).c_str(),0);
   
   idx = resp.find_first_not_of(" ", edx);
   edx = resp.find_first_of(" ", idx+1);
   if(idx < 0 || edx < 0 || edx < idx) return not_connected(); 
   ksize = atoi(resp.substr(idx, edx-idx).c_str());
   
   idx = resp.find_first_not_of(" ", edx);
   edx = resp.find_first_of(" ", idx+1);
   if(idx < 0 || edx < 0 || edx < idx) return not_connected();      
   useAvg = atoi(resp.substr(idx, edx-idx).c_str());

   idx = resp.find_first_not_of(" ", edx);
   edx = resp.find_first_of(" ", idx+1);
   if(idx < 0 || edx < 0 || edx < idx) return not_connected();      
   minMaxRej = atoi(resp.substr(idx, edx-idx).c_str());

   idx = resp.find_first_not_of(" ", edx);
   edx = resp.find_first_of(" ", idx+1);
   if(idx < 0 ) return not_connected();  
   avgLen = atoi(resp.substr(idx, edx-idx).c_str());

   state_connected = 1;
   return;
}

void CoronGuideForm::not_connected()
{
   state_connected = 0;
   
   
   return;
}

void CoronGuideForm::update_status()
{
   char tmpstr[256];
   
   if(!state_connected)
   {
      ui.labelLoopState->setText("unknown");
      ui.labelLoopGain->setText("?");
      ui.labelBGAlgo->setText("?");
      
      //ui.kfwhmCurr->setText("?");
      
      ui.buttonClosePause->setEnabled(false);
      ui.buttonOpen->setEnabled(false);
      ui.gainSpinBox->setEnabled(false);
      ui.buttonSetGain->setEnabled(false);
      
      return;
   }
   
   if(loop_state == 0) 
   {
      ui.labelLoopState->setText("OPEN"); 
      ui.buttonClosePause->setText("close");
   }
   else if(loop_state == 2) 
   {
      ui.labelLoopState->setText("PAUSED");
      ui.buttonClosePause->setText("re-close");
   }
   else 
   {
      ui.buttonClosePause->setText("pause");
      if(loop_state == 1) ui.labelLoopState->setText("CLOSED");
      if(loop_state == 3) ui.labelLoopState->setText("AUTOPAUSE");
      if(loop_state == 4) ui.labelLoopState->setText("AP RECOVERY");
   }
   
   snprintf(tmpstr, 256, "%0.2f", loop_gain);
   ui.labelLoopGain->setText(tmpstr);
   
   //snprintf(tmpstr, 256, "%0.2f", kfwhm);
   //ui.kfwhmCurr->setText(tmpstr);
   
   //ui.bgAlgoCombo->setCurrentIndex(bgAlgorithm);
   
   switch(bgAlgorithm)
   {
      case 0:
         ui.labelBGAlgo->setText("median sub");
         break;
      default:
         snprintf(tmpstr, 256, "unsharp FWHM=%0.2f", kfwhm);
         ui.labelBGAlgo->setText(tmpstr);
   }
   
   if(useAvg)
   {
      if(minMaxRej)
      {
         snprintf(tmpstr, 256, "%i, rejecting min/max", avgLen);
      }
      else
      {
         snprintf(tmpstr, 256, "%i, not rejecting", avgLen);
      }
      ui.labelAvgCurr->setText(tmpstr);
   }
   else
   {
      ui.labelAvgCurr->setText("no");
   }
   
   if(state_cmode != 'L')
   {
      ui.buttonClosePause->setEnabled(false);
      ui.buttonOpen->setEnabled(false);
      ui.gainSpinBox->setEnabled(false);
      ui.buttonSetGain->setEnabled(false);
   }  
   else
   {
      ui.buttonClosePause->setEnabled(true);
      
      if(loop_state == 0)
      {
         ui.buttonOpen->setEnabled(false);
      }
      else
      {
         ui.buttonOpen->setEnabled(true);
      }
      
      ui.gainSpinBox->setEnabled(true);
      ui.buttonSetGain->setEnabled(true);
   }
                                 
   
   
}


void CoronGuideForm::on_buttonClosePause_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   
   if(loop_state == 0 || loop_state == 2)
   {
      write_fifo_channel(CG_FIFO_CH, "close", 5, &resp);
   }
   else
   {
      write_fifo_channel(CG_FIFO_CH, "pause", 5, &resp);
   }
   
   pthread_mutex_unlock(&commutex);
   
   statustimerout();
}

void CoronGuideForm::on_buttonOpen_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   
   write_fifo_channel(CG_FIFO_CH, "open", 4, &resp);
   
   pthread_mutex_unlock(&commutex);
   
   statustimerout();
}

void CoronGuideForm::on_buttonSetGain_clicked()
{
   std::string resp;
   char tmpstr[64];
   
   snprintf(tmpstr, 64, "gain %0.4f", ui.gainSpinBox->value());
   
   pthread_mutex_lock(&commutex);
   
   write_fifo_channel(CG_FIFO_CH, tmpstr, strlen(tmpstr), &resp);
   
   pthread_mutex_unlock(&commutex);
   
   statustimerout();
}
   
void CoronGuideForm::on_kfwhmSetButton_clicked()
{
   std::string resp;
   char tmpstr[64];
   
   snprintf(tmpstr, 64, "kfwhm %0.4f", ui.kfwhmEntry->text().toDouble());
   
   pthread_mutex_lock(&commutex);
   
   write_fifo_channel(CG_FIFO_CH, tmpstr, strlen(tmpstr), &resp);
   
   pthread_mutex_unlock(&commutex);
   
   statustimerout();
}     

void CoronGuideForm::on_bgAlgoCombo_activated(int k)
{
   
   std::string resp;
   char tmpstr[64];
   
   snprintf(tmpstr, 64, "bgalgo %i", k-1);
   
   pthread_mutex_lock(&commutex);
   
   write_fifo_channel(CG_FIFO_CH, tmpstr, strlen(tmpstr), &resp);
   
   pthread_mutex_unlock(&commutex);
   
   ui.bgAlgoCombo->setCurrentIndex(0);
   
   
}

void CoronGuideForm::on_avgSetButton_clicked()
{
   std::string resp;
   char tmpstr[64];
   
   pthread_mutex_lock(&commutex);
   
   snprintf(tmpstr, 64, "useavg %i", ui.useAvgCheck->checkState() == Qt::Checked);
   write_fifo_channel(CG_FIFO_CH, tmpstr, strlen(tmpstr), &resp);

   snprintf(tmpstr, 64, "mmrej %i", ui.minMaxCheck->checkState() == Qt::Checked);
   write_fifo_channel(CG_FIFO_CH, tmpstr, strlen(tmpstr), &resp);
   
   snprintf(tmpstr, 64, "avglen %i", ui.numAvgEntry->text().toInt());
   write_fifo_channel(CG_FIFO_CH, tmpstr, strlen(tmpstr), &resp);
   
   pthread_mutex_unlock(&commutex);
         
}

void CoronGuideForm::on_ButtonTakeLocal_clicked()
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
