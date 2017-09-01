#include "basicfilterwheelform.h"

namespace VisAO
{

BasicFilterWheelForm::BasicFilterWheelForm(QWidget * Parent, Qt::WindowFlags f) : basic_ui(Parent,f)
{
};

BasicFilterWheelForm::BasicFilterWheelForm(int argc, char **argv, QWidget * Parent, Qt::WindowFlags f) : basic_ui(argc, argv, Parent, f)
{
   Create();
}

BasicFilterWheelForm::BasicFilterWheelForm(std::string name, const std::string& conffile, QWidget * Parent, Qt::WindowFlags f) : basic_ui(name, conffile, Parent, f)
{
   Create();
}

void BasicFilterWheelForm::Create()
{
   ui.setupUi(this);	
   attach_basic_ui();
   
   offset = 0.0;
   
   ui.lineEditPosition->setFocusPolicy(Qt::ClickFocus);

   try
   {
      FilterWheel_name = (std::string)(ConfigDictionary())["FilterWheel_name"];
   }
   catch(Config_File_Exception &e)
   {
      log_msg(Logger::LOG_LEV_FATAL, "No filter wheel name (FilterWheel_name) specified.");
      std::cerr << "No filter wheel name (FilterWheel_name) specified. (logged)\n";
      throw;
   }

   

   try
   {
      fw_name = Utils::getConffile(FilterWheel_name);

      fw_conf = new Config_File(fw_name);
   }
   catch(Config_File_Exception &e)
   {
      logss.str("");
      logss << "Error accessing filter wheel configuration files.";
      log_msg(Logger::LOG_LEV_FATAL, logss.str());
      std::cerr << logss.str() << " (logged) \n";
      throw;
   }

   setup_fifo_list(1);

   std::string visao_root = getenv("VISAO_ROOT");
   
   std::string FilterWheel_fifopath = (std::string)(ConfigDictionary())["FilterWheel_fifopath"];

   char * side = getenv("ADOPT_SIDE");
   //sprintf(pnum, "%02i", (int)(*fw_conf)["ID"]);
   //std::string fpathin = visao_root + "/" + FocusMotor_fifopath + "/" + ((std::string) (*fw_conf)["MyName"]) + pnum + "_com_local_in";
   //std::string fpathout = visao_root + "/" + FocusMotor_fifopath + "/" + ((std::string) (*fw_conf)["MyName"]) + pnum + "_com_local_out";

   FilterWheel_name += "Local";
   
   std::string fpathin = visao_root + "/" + FilterWheel_fifopath + "/" + FilterWheel_name + "_com_local_in";
   std::string fpathout = visao_root + "/" + FilterWheel_fifopath + "/" + FilterWheel_name + "_com_local_out";

   // Read custom position list, if present.
   customPos.clear();
   
   int numCustomPos=0;
   try
   {
      numCustomPos = (*fw_conf)["customPositionNum"];
   }
   catch (Config_File_Exception &e)
   {
      _logger->log( Logger::LOG_LEV_INFO, "No custom positions defined in cfg file.");
   }
   
   if (numCustomPos >0)
   {
      for (int i=0; i<numCustomPos; i++)
      {
         ostringstream namekey, poskey;
         namekey << "pos" << i << "_name";
         poskey << "pos" << i << "_pos";
         string name;
         float pos;
         try
         {
            name = (std::string) (*fw_conf)[namekey.str()];
            pos  = (float) (*fw_conf)[poskey.str()];
            customPosName.push_back(name);
            customPos.push_back(pos);
         }
         catch (Config_File_Exception &e)
         {
            _logger->log( Logger::LOG_LEV_ERROR, "Custom position %d not found in cfg file.", i);
         }
      }
   }

   ui.comboFilters->insertItem(0, "");
   for(int i=0; i < numCustomPos; i++)
   {
      ui.comboFilters->insertItem(i+1, customPosName[i].c_str());
      //std::cout << customPosName[i] << " " << customPos[i] << "\n";
   }

   ui.comboFilters->setLayoutDirection(Qt::LeftToRight);
   
   //Set gui title
   try
   {
      std::string gid =  (std::string)(ConfigDictionary())["GUI_ID"];
      std::string newtitle = "Filter Wheel ";
      newtitle += gid;
      ui.title->setText(newtitle.c_str());
   }
   catch(Config_File_Exception &e)
   {
      //no worries
   }
   signal(SIGIO, SIG_IGN);
   set_fifo_list_channel(&fl, 0, 100, fpathout.c_str(), fpathin.c_str(), 0, 0);

   //global_fifo_list = &fl;

   if(connect_fifo_list() < 0)
   {
      QMessageBox msgBox;
      msgBox.setText("Another Filter Wheel GUI is probably already running.");
      msgBox.setInformativeText("Cannot lock the input FIFO on one or more channels.");
      msgBox.exec();
      exit(-1);
   }

   wait_to = .2;

   ui.statusOther->setText("");
   statustimerout();

   ui.currentFilter->setStyleSheet(paramStyle);
   ui.currentPosition->setStyleSheet(paramStyle);
   
   return;
}

void BasicFilterWheelForm::attach_basic_ui()
{
   __LabelCmode = ui.LabelCmode;
   __StatusCmode = ui.StatusCmode;
   __StatusConnected = ui.StatusConnected;
   __ButtonTakeLocal = ui.ButtonTakeLocal;
   __checkBoxOverride = ui.checkBoxOverride;

   ui.cModeFrame->lower();
}

void BasicFilterWheelForm::retrieve_state()
{
   std::string resp;

   //std::cout << "Getting state . . .\n";
   write_fifo_channel(0, "state?\n", 8, &resp);
   //std::cout << "State resp: " << resp << "\n";

   if(resp == "" || resp.length() < 20)
   {
      if(resp == "")
      {
         //std::cout << "0 response\n";
      }
      else
      {
         //std::cout << "Response too short: " << resp.length() << " " << resp << "\n";
         return;
      }

      state_connected = 0;
      state_cmode = -1;
      state_pos = -1e12;
      state_is_moving = -2;
      
      return;
   }

   state_connected = 1;
   
   state_cmode = resp[0];
   
   state = atoi(resp.substr(2,2).c_str());

   state_is_moving = atoi(resp.substr(5,1).c_str());

   state_homing = atoi(resp.substr(7,1).c_str());

   state_pos = strtod(resp.substr(9, 9).c_str(), 0);
   state_filter = resp.substr(19, resp.length()-19-1);

   /*std::cout << (char) state_cmode << "\n";
   std::cout << state << "\n";
   std::cout << state_is_moving << "\n";
   std::cout << state_homing << "\n";
   std::cout << state_pos << "\n";
   std::cout << state_filter << "\n";*/

   return;

}

void BasicFilterWheelForm::update_status()
{
  

   switch(state)
   {
      case STATE_NOCONNECTION:
         ui.state->setText("no connection");
         break;
      case STATE_READY:
         ui.state->setText("ready");
         break;
      case STATE_OPERATING:
         ui.state->setText("moving");
         break;
      case STATE_HOMING:
         ui.state->setText("homing");
         break;
      default:
         ui.state->setText("unkown");
         break;
   }

   if(state_connected == 0)
   {
      ui.state->setText("");
      ui.currentFilter->setText("unkown");
      ui.currentPosition->setText("");
   }
   else
   {
      char posstr[12];
      snprintf(posstr, 12, "%0.4f", state_pos);
   
      ui.currentPosition->setText(posstr);
      ui.currentFilter->setText(state_filter.c_str());
   }

   if(state_connected == 0 || state_cmode != 'L')
   {
      ui.ButtonGO->setEnabled(false);
      ui.ButtonGOFilter->setEnabled(false);
      ui.ButtonPos->setEnabled(false);
      ui.ButtonNeg->setEnabled(false);
      ui.ButtonAbort->setEnabled(true);
      ui.ButtonHome->setEnabled(false);
      return;
   }
   
   if(state_is_moving && !state_homing)
   {
      ui.lineEditPosition->clearFocus();
      ui.ButtonGO->setEnabled(false);
      ui.ButtonGOFilter->setEnabled(false);
      ui.ButtonPos->setEnabled(false);
      ui.ButtonNeg->setEnabled(false);
      ui.ButtonAbort->setEnabled(true);
      ui.ButtonHome->setEnabled(false);
      return;
   }

   if(state_homing)
   {
      ui.lineEditPosition->clearFocus();
      ui.ButtonGO->setEnabled(false);
      ui.ButtonGOFilter->setEnabled(false);
      ui.ButtonPos->setEnabled(false);
      ui.ButtonNeg->setEnabled(false);
      ui.ButtonAbort->setEnabled(true);
      ui.ButtonHome->setEnabled(false);
      return;
   }

   ui.ButtonGO->setEnabled(true);
   ui.ButtonGOFilter->setEnabled(true);
   ui.ButtonPos->setEnabled(true);
   ui.ButtonNeg->setEnabled(true);
   ui.ButtonAbort->setEnabled(true);
   ui.ButtonHome->setEnabled(true);
   
}

void BasicFilterWheelForm::on_ButtonGO_clicked()
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

void BasicFilterWheelForm::on_ButtonGOFilter_clicked()
{
   std::string resp;
   int idx;
   char posstr[20];

   idx = ui.comboFilters->currentIndex();
   if(idx == 0) return;

   ui.lineEditPosition->clearFocus();
   snprintf(posstr, 20, "pos %0.3f\n", customPos[idx-1]);

   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, posstr, strlen(posstr), &resp);
   pthread_mutex_unlock(&commutex);
   
   ui.comboFilters->setCurrentIndex(0);
}

void BasicFilterWheelForm::on_ButtonAbort_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, "abort\n", 7, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicFilterWheelForm::on_ButtonHome_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   //std::cout << "Sending Home  . . .\n";
   //sleep(10);
   write_fifo_channel(0, "home\n", 6, &resp);
   //std::cout << "Home resp =" << resp << "\n";
   //sleep(10);
   pthread_mutex_unlock(&commutex);
}

void BasicFilterWheelForm::on_ButtonPos_clicked()
{
   std::string resp;
   char posstr[20];
   if(!state_is_moving && state_cmode == 'L')
   {
      double newpos = state_pos + offset;

      snprintf(posstr, 20, "pos %0.4f\n", newpos);

      pthread_mutex_lock(&commutex);
      write_fifo_channel(0, posstr, strlen(posstr), &resp);
      pthread_mutex_unlock(&commutex);
      
   }
}

void BasicFilterWheelForm::on_ButtonNeg_clicked()
{
   std::string resp;
   char posstr[20];
   if(!state_is_moving && state_cmode == 'L')
   {
      double newpos = state_pos - offset;

      snprintf(posstr, 20, "pos %0.4f\n", newpos);

      pthread_mutex_lock(&commutex);
      write_fifo_channel(0, posstr, strlen(posstr), &resp);
      pthread_mutex_unlock(&commutex);
   }
}

void BasicFilterWheelForm::on_lineEditOffset_editingFinished()
{
   offset = ui.lineEditOffset->text().toDouble();
}

void BasicFilterWheelForm::on_ButtonTakeLocal_clicked()
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

