#include "basicgimbalform.h"

#include <iostream>

namespace VisAO
{

//#define _debug

BasicGimbalForm::BasicGimbalForm(QWidget * Parent, Qt::WindowFlags f) : basic_ui(Parent, f)
{

}

BasicGimbalForm::BasicGimbalForm(int argc, char **argv, QWidget * Parent, Qt::WindowFlags f) : basic_ui(argc, argv, Parent, f)
{
   Create();
}

BasicGimbalForm::BasicGimbalForm(std::string name, const std::string& conffile, QWidget * Parent, Qt::WindowFlags f) : basic_ui(name, conffile, Parent, f)
{
   Create();
}

void BasicGimbalForm::Create()
{
   ui.setupUi(this);
   attach_basic_ui();

   int autodelay = 25;
   int autorepeat = 25;
   
   upPressed = false;
   ui.upButton->setAutoRepeat(true);
   ui.upButton->setAutoRepeatDelay(autodelay);
   ui.upButton->setAutoRepeatInterval(autorepeat);

   downPressed = false;
   ui.downButton->setAutoRepeat(true);
   ui.downButton->setAutoRepeatDelay(autodelay);
   ui.downButton->setAutoRepeatInterval(autorepeat);

   leftPressed = false;
   ui.leftButton->setAutoRepeat(true);
   ui.leftButton->setAutoRepeatDelay(autodelay);
   ui.leftButton->setAutoRepeatInterval(autorepeat);

   rightPressed = false;
   ui.rightButton->setAutoRepeat(true);
   ui.rightButton->setAutoRepeatDelay(autodelay);
   ui.rightButton->setAutoRepeatInterval(autorepeat);

   xMoving = false;
   yMoving = false;


   //bigMove = 1.5;

   
   
   setup_fifo_list(1);
   
   std::string visao_root = getenv("VISAO_ROOT");
   std::string gimbal_fifopath;

   try
   {
      gimbal_fifopath = (std::string)(ConfigDictionary())["gimbal_fifopath"];
   }
   catch(...)
   {
      gimbal_fifopath = "fifos";
   }
   
   std::string fpathin = visao_root + "/" + gimbal_fifopath + "/gimbal_com_local_in";
   std::string fpathout = visao_root + "/" + gimbal_fifopath + "/gimbal_com_local_out";

   
   signal(SIGIO, SIG_IGN);
   set_fifo_list_channel(&fl, 0, 100,fpathout.c_str(), fpathin.c_str(), 0, 0);
   
   if(connect_fifo_list() < 0)
   {
      QMessageBox msgBox;
      msgBox.setText("Another Gimbal GUI is probably already running.");
      msgBox.setInformativeText("Cannot lock the input FIFO on one or more channels.");
      msgBox.exec();
      exit(-1);
   }

   wait_to = 1.;

   try
   {
      xLimit = (double)(ConfigDictionary())["xLimit"];
   }
   catch(...)
   {
      xLimit = 3.5;
   }

   try
   {
      yLimit = (double)(ConfigDictionary())["yLimit"];
   }
   catch(...)
   {
      yLimit = 3.5;
   }

   try
   {
      slew_lag = (double)(ConfigDictionary())["slew_lag"];
   }
   catch(...)
   {
      slew_lag = 0.5;
   }

   if(slew_lag < 0.5) slew_lag = 0.5;

   try
   {
      small_move = (double)(ConfigDictionary())["small_move"];
   }
   catch(...)
   {
      small_move = 0.5;
   }

   statustimerout();
   
   QString style = "background-color : lightgrey; color : black; ";
   ui.currX->setStyleSheet(style);
   ui.currY->setStyleSheet(style);
         
   
   return;
}

void BasicGimbalForm::attach_basic_ui()
{
   __LabelCmode = ui.LabelCmode;
   __StatusCmode = ui.StatusCmode;
   __StatusConnected = ui.StatusConnected;
   __ButtonTakeLocal = ui.ButtonTakeLocal;
   __checkBoxOverride = ui.checkBoxOverride;

   ui.cModeFrame->lower();
}

void BasicGimbalForm::retrieve_state()
{
   //double startT;
   int spos, epos;
   std::string resp, tmp;
   
   write_fifo_channel(0, "state?\n", 8, &resp);

   if(resp.length() < 5)
   {
      return; //This means we got somebody elses response.
            //This seems to happen due ot the autorepeater stuff.
   }
   //startT = get_curr_time();
   if(resp == "" || resp.length()<5)
   {
      state_connected = 0;
      state_cmode = -1;      
      return;
   }

   state_connected = 1;
      
   state_cmode = resp[0];

   spos = resp.find(',', 0);
   epos = resp.find(',', spos+1);

   tmp = resp.substr(spos+1, epos-spos-1);

   curState = atoi(tmp.c_str());

   spos = resp.find(',', epos);
   epos = resp.find(',', spos+1);
  
   pwrState = atoi(tmp.c_str());

   spos = resp.find(',', epos);
   epos = resp.find(',', spos+1);
   
   tmp = resp.substr(spos+1, epos-spos-1);
   
   //xMoving = atoi(tmp.c_str());

   spos = resp.find(',', epos);
   epos = resp.find(',', spos+1);
   
   tmp = resp.substr(spos+1, epos-spos-1);
   
   xPos = strtod(tmp.c_str(),0);

   //** y moving **//
   spos = resp.find(',', epos);
   epos = resp.find(',', spos+1);
   
   tmp = resp.substr(spos+1, epos-spos-1);
   
   //yMoving = atoi(tmp.c_str());

   //** y position **//
   spos = resp.find(',', epos);
   epos = resp.find(',', spos+1);
   if(epos == -1) epos = resp.length();
   
   tmp = resp.substr(spos+1, epos-spos-1);
   
   yPos = strtod(tmp.c_str(),0);

   //** Scale ** //
   spos = resp.find(',', epos);
   epos = resp.find(',', spos+1);
   if(epos == -1) epos = resp.length();
   
   tmp = resp.substr(spos+1, epos-spos-1);
   
   scale = strtod(tmp.c_str(),0);
   
   return;
   
}
void BasicGimbalForm::update_status()
{
   char str[256];
   
   switch(curState)
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

   if(curState != STATE_NOCONNECTION && curState != STATE_INVALID && curState != STATE_ERROR && curState != STATE_OFF)
   {
      snprintf(str, 256, "%0.5f", xPos);
      ui.currX->setText(str);

      snprintf(str, 256, "%0.5f", yPos);
      ui.currY->setText(str);
      
      if(state_cmode == 'L')
      {
         ui.upButton->setEnabled(true);
         ui.leftButton->setEnabled(true);
         ui.rightButton->setEnabled(true);
         ui.downButton->setEnabled(true);
         ui.goXButton->setEnabled(true);
         ui.goYButton->setEnabled(true);
         ui.centerButton->setEnabled(true);
         ui.darkButton->setEnabled(true);
         ui.savePresetButton->setEnabled(true);
      }
      else
      {
         ui.upButton->setEnabled(false);
         ui.leftButton->setEnabled(false);
         ui.rightButton->setEnabled(false);
         ui.downButton->setEnabled(false);
         ui.goXButton->setEnabled(false);
         ui.goYButton->setEnabled(false);
         ui.centerButton->setEnabled(false);
         ui.darkButton->setEnabled(false);
         ui.savePresetButton->setEnabled(false);
      }
   }
   else
   {
      ui.currX->setText("?");
      ui.currY->setText("?");
      
      ui.upButton->setEnabled(false);
      ui.leftButton->setEnabled(false);
      ui.rightButton->setEnabled(false);
      ui.downButton->setEnabled(false);
      ui.goXButton->setEnabled(false);
      ui.goYButton->setEnabled(false);
      ui.centerButton->setEnabled(false);
      ui.darkButton->setEnabled(false);
      ui.savePresetButton->setEnabled(false);
   }
   
}

void BasicGimbalForm::on_ButtonTakeLocal_clicked()
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

void BasicGimbalForm::on_upButton_pressed()
{
   double dt;
   std::string resp;
   char str[256];
   
   if(upPressed)
   {
      dt = get_curr_time() - timeUpPressed;
      if(dt > slew_lag && !yMoving)
      {
         double dm = 0. - yPos + .0001;
         #ifdef _debug
         std::cout << "Start Up Move " << dm << "\n";
         #endif
         snprintf(str, 256, "yrel %f", dm);
         
         pthread_mutex_lock(&commutex);
         write_fifo_channel(0, str, strlen(str)+1, &resp);
         pthread_mutex_unlock(&commutex);
         
         yMoving = true;
      }
   }
   else
   {
      #ifdef _debug
      std::cout << "Up Pressed\n";
      #endif
      
      timeUpPressed = get_curr_time();
      upPressed = true;
   }
   
}

void BasicGimbalForm::on_upButton_released()
{
   double dt;
   std::string resp;
   char str[256];

   if(ui.upButton->isDown()) return;

   dt = get_curr_time() - timeUpPressed;

   if(yMoving)
   {
      #ifdef _debug
      std::cout << "Stop Up Move " << dt << "\n";
      #endif
      
      pthread_mutex_lock(&commutex);
      write_fifo_channel(0, "stopy", 5, &resp);
      pthread_mutex_unlock(&commutex);
      yMoving = false;
   }
   else
   {
      double dm = -1.*dt/(0.5)*small_move;
      if(dm < 0. - yPos) dm = 0. - yPos + .0001;

      #ifdef _debug
      std::cout << "Do Up Move " << dt << " " << dm << "\n";
      #endif
      
      snprintf(str, 256, "yrel %f", dm);
      pthread_mutex_lock(&commutex);
      write_fifo_channel(0, str, strlen(str)+1, &resp);
      pthread_mutex_unlock(&commutex);
   }
   upPressed = false;
   
}

void BasicGimbalForm::on_downButton_pressed()
{
   double dt;
   std::string resp;
   char str[256];
   if(downPressed)
   {
      dt = get_curr_time() - timeDownPressed;
      if(dt > slew_lag && !yMoving)
      {
         double dm = yLimit*scale - yPos - .0001*scale;

         #ifdef _debug
         std::cout << "Start Down Move " << dm << "\n";
         #endif
         snprintf(str, 256, "yrel %f", dm);
         pthread_mutex_lock(&commutex);
         write_fifo_channel(0, str, strlen(str)+1, &resp);
         pthread_mutex_unlock(&commutex);
         
         yMoving = true;
      }
   }
   else
   {
      #ifdef _debug
      std::cout << "Down Pressed\n";
      #endif
      timeDownPressed = get_curr_time();
      downPressed = true;
   }
   
}

void BasicGimbalForm::on_downButton_released()
{
   double dt;
   std::string resp;
   char str[256];
   if(ui.downButton->isDown()) return;
   
   dt = get_curr_time() - timeDownPressed;
   
   if(yMoving)
   {
      #ifdef _debug
      std::cout << "Stop Down Move " << dt << "\n";
      #endif
      pthread_mutex_lock(&commutex);
      write_fifo_channel(0, "stopy", 5, &resp);
      pthread_mutex_unlock(&commutex);
      yMoving = false;
   }
   else
   {
      double dm = dt/(0.5)*small_move;
      if(dm > yLimit*scale - yPos) dm = yLimit*scale - yPos - .0001*scale;
      #ifdef _debug
      std::cout << "Do Down Move " << dt << " " << dm << "\n";
      #endif
      snprintf(str, 256, "yrel %f", dm);
      pthread_mutex_lock(&commutex);
      write_fifo_channel(0, str, strlen(str)+1, &resp);
      pthread_mutex_unlock(&commutex);
   }
   downPressed = false;
   
}

void BasicGimbalForm::on_leftButton_pressed()
{
   double dt;
   std::string resp;
   char str[256];

   if(leftPressed)
   {
      dt = get_curr_time() - timeLeftPressed;
      if(dt > slew_lag && !xMoving)
      {
         double dm = 0. - xPos + .0001;
         #ifdef _debug
         std::cout << "Start Left Move " << dm << "\n";
         #endif
         snprintf(str, 256, "xrel %f", dm);
         pthread_mutex_lock(&commutex);
         write_fifo_channel(0, str, strlen(str)+1, &resp);
         pthread_mutex_unlock(&commutex);
         xMoving = true;
      }
   }
   else
   {
      #ifdef _debug
      std::cout << "Left Pressed\n";
      #endif
      timeLeftPressed = get_curr_time();
      leftPressed = true;
   }
   
}

void BasicGimbalForm::on_leftButton_released()
{
   char str[256];
   std::string resp;
   
   double dt;
   
   if(ui.leftButton->isDown()) return;
   
   dt = get_curr_time() - timeLeftPressed;
   
   if(xMoving)
   {
      #ifdef _debug
      std::cout << "Stop Left Move " << dt << "\n";
      #endif
      pthread_mutex_lock(&commutex);
      write_fifo_channel(0, "stopx", 5, &resp);
      pthread_mutex_unlock(&commutex);
      xMoving = false;      
   }
   else
   {
      double dm = -1.*dt/(0.5)*small_move;
      if(dm < 0. - xPos) dm = 0. - xPos + .0001;
      #ifdef _debug
      std::cout << "Do Left Move " << dt << " " << dm << "\n";
      #endif
      
      snprintf(str, 256, "xrel %f", dm);
      
      pthread_mutex_lock(&commutex);
      write_fifo_channel(0, str, strlen(str)+1, &resp);
      pthread_mutex_unlock(&commutex);
      
   }
   
   leftPressed = false;

}

void BasicGimbalForm::on_rightButton_pressed()
{
   char str[256];
   std::string resp;
   
   double dt;
   
   if(rightPressed)
   {
      dt = get_curr_time() - timeRightPressed;
      if(dt > slew_lag && !xMoving)
      {
         double dm = xLimit*scale - xPos - .0001*scale;
         #ifdef _debug
         std::cout << "Start Right Move " << dm << "\n";
         #endif
         
         snprintf(str, 256, "xrel %f", dm);
         pthread_mutex_lock(&commutex);
         write_fifo_channel(0, str, strlen(str)+1, &resp);
         pthread_mutex_unlock(&commutex);
         
         xMoving = true;
         
      }
   }
   else
   {
      #ifdef _debug
      std::cout << "Right Pressed\n";
      #endif
      
      timeRightPressed = get_curr_time();
      rightPressed = true;
   }
   
}

void BasicGimbalForm::on_rightButton_released()
{
   char str[256];
   std::string resp;
   
   double dt;
   
   if(ui.rightButton->isDown()) return;
   
   dt = get_curr_time() - timeRightPressed;
   
   if(xMoving)
   {
      #ifdef _debug
      std::cout << "Stop Right Move " << dt << "\n";
      #endif
      
      pthread_mutex_lock(&commutex);
      write_fifo_channel(0, "stopx", 5, &resp);
      pthread_mutex_unlock(&commutex);
      
      xMoving = false;
   }
   else
   {
      double dm = dt/(0.5)*small_move;

      #ifdef _debug
      std::cout << "Do Right Move " << dt << " " << dm << "\n";
      #endif
      
      if(dm > xLimit*scale - xPos) dm = xLimit*scale - xPos - .0001*scale;
      
      snprintf(str, 256, "xrel %f", dm);
      pthread_mutex_lock(&commutex);
      write_fifo_channel(0, str, strlen(str)+1, &resp);
      pthread_mutex_unlock(&commutex);
   }
   rightPressed = false;
   
}

void BasicGimbalForm::on_stopButton_clicked()
{
   std::string resp;
   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, "stop", 5, &resp);
   pthread_mutex_unlock(&commutex);

}


void BasicGimbalForm::on_centerButton_clicked()
{
   std::string resp;
   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, "center", 7, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicGimbalForm::on_darkButton_clicked()
{
   std::string resp;
   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, "dark", 5, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicGimbalForm::on_goXButton_clicked()
{
   char str[256];
   std::string resp;
   double newx;

   newx = ui.newX->text().toDouble();

   if(newx < 0) newx = 0.;
   if(newx > xLimit*scale) newx = xLimit*scale;
   
   snprintf(str, 256, "xabs %f", newx);
   
   #ifdef _debug
   std::cout << "Go X clicked " << newx << "\n";
   #endif

   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, str, strlen(str)+1, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicGimbalForm::on_goYButton_clicked()
{
   char str[256];
   std::string resp;
   double newy;
   
   newy = ui.newY->text().toDouble();

   if(newy < 0) newy = 0.;
   if(newy > yLimit*scale) newy = yLimit*scale;
   
   snprintf(str, 256, "yabs %f", newy);
   #ifdef _debug
   std::cout << "Go Y clicked " << newy << " " << str << "\n";
   #endif
   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, str, strlen(str)+1, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicGimbalForm::on_savePresetButton_clicked()
{
   std::string resp;
   
   pthread_mutex_lock(&commutex);

   write_fifo_channel(0, "savepreset\n", 12, &resp);
   
   pthread_mutex_unlock(&commutex);
}

} //namespace VisAO
