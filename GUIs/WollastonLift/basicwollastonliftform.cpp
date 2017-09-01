#include "basicwollastonliftform.h"

#include <iostream>

namespace VisAO
{

BasicWollastonLiftForm::BasicWollastonLiftForm(QWidget * Parent, Qt::WindowFlags f) : basic_ui(Parent, f)
{

}

BasicWollastonLiftForm::BasicWollastonLiftForm(int argc, char **argv, QWidget * Parent, Qt::WindowFlags f) : basic_ui(argc, argv, Parent, f)
{
   Create();
}

BasicWollastonLiftForm::BasicWollastonLiftForm(std::string name, const std::string& conffile, QWidget * Parent, Qt::WindowFlags f) : basic_ui(name, conffile, Parent, f)
{
   Create();
}

void BasicWollastonLiftForm::Create()
{
   ui.setupUi(this);
   attach_basic_ui();
   
   setup_fifo_list(1);
   
   std::string visao_root = getenv("VISAO_ROOT");
   std::string WollastonStatus_fifopath = (std::string)(ConfigDictionary())["WollastonStatus_fifopath"];
   
   std::string fpathin = visao_root + "/" + WollastonStatus_fifopath + "/wollastonstatus_com_local_in";
   std::string fpathout = visao_root + "/" + WollastonStatus_fifopath + "/wollastonstatus_com_local_out";
   
   signal(SIGIO, SIG_IGN);
   set_fifo_list_channel(&fl, 0, 100,fpathout.c_str(), fpathin.c_str(), 0, 0);
   
   if(connect_fifo_list() < 0)
   {
      QMessageBox msgBox;
      msgBox.setText("Another Wollaston Status GUI is probably already running.");
      msgBox.setInformativeText("Cannot lock the input FIFO on one or more channels.");
      msgBox.exec();
      exit(-1);
   }

   prompt_msg_open = false;
   prompt_dead_time = get_curr_time();
   wait_to = 1.;
   //write_fifo_channel(0, "LOCAL\n", 6);
   
   statustimerout();
   
   return;
}

void BasicWollastonLiftForm::attach_basic_ui()
{
   __LabelCmode = ui.LabelCmode;
   __StatusCmode = ui.StatusCmode;
   __StatusConnected = ui.StatusConnected;
   __ButtonTakeLocal = ui.ButtonTakeLocal;
   __checkBoxOverride = ui.checkBoxOverride;

   ui.cModeFrame->lower();
}

void BasicWollastonLiftForm::retrieve_state()
{
   //double startT;
   std::string resp;
   
   write_fifo_channel(0, "state?\n", 8, &resp);
   
   //startT = get_curr_time();
   
   if(resp == "")
   {
      state_connected = 0;
      state_cmode = -1;
      state_pos = -2;
      state_prompt = 0;
      
      return;
   }
   
   state_connected = 1;
   
   
   if(resp.length() > 2)
   {
      state_cmode = resp[0];
      
      state_pos = atoi(resp.substr(2,2).c_str());

      state_prompt = atoi(resp.substr(4,resp.length()-4).c_str());
   }
   return;
   
}
void BasicWollastonLiftForm::update_status()
{
   if(state_connected == 0 || state_cmode != 'L')
   {
      if(state_pos == 0)
      {
         ui.statusStatus->setText("INT");
      }
      if(state_pos == 1) ui.statusStatus->setText("IN");
      if(state_pos == -1) ui.statusStatus->setText("OUT");
      if(state_pos == -2) ui.statusStatus->setText("UNK");
      ui.upButton->setEnabled(false);
      ui.downButton->setEnabled(false);

      check_prompt();
      return;
   }
   
   if(state_pos == 1)
   {
      ui.statusStatus->setText("IN");
      ui.upButton->setEnabled(false);
      ui.downButton->setEnabled(true);
      check_prompt();
      return;
   }
   
   if(state_pos == -1)
   {
      ui.statusStatus->setText("OUT");
      ui.upButton->setEnabled(true);
      ui.downButton->setEnabled(false);
      check_prompt();
      return;
   }
   
   if(state_pos == 0)
   {
      ui.statusStatus->setText("INT");
      ui.upButton->setEnabled(true);
      ui.downButton->setEnabled(true);
      check_prompt();
      return;
   }
   
   ui.statusStatus->setText("UNK");
   ui.upButton->setEnabled(true);
   ui.downButton->setEnabled(true);

   check_prompt();
}

void BasicWollastonLiftForm::check_prompt()
{
   if(state_prompt <= 0) return;
   if(prompt_msg_open) return;
   if(get_curr_time()-prompt_dead_time < 10.) return;
   
   std::string resp;
   QMessageBox msgBox;
   msgBox.setWindowModality(Qt::ApplicationModal);
   msgBox.setWindowTitle("Wollaston Status");
   
   msgBox.setIcon(QMessageBox::Question);
   
   if(state_cmode != 'L')   msgBox.setInformativeText("You must take local control to change state.");
   
   if(state_prompt == PROMPT_UP)
   {      
      QAbstractButton *yesButton = msgBox.addButton("Yes", QMessageBox::YesRole);
      QAbstractButton *noButton = msgBox.addButton("No", QMessageBox::HelpRole);
      
      msgBox.setText("The Wollaston state is OUT but you have selected an SDI filter.  Do you wish to set the Wollaston state to IN?");
      prompt_msg_open = true;
      msgBox.exec();
      prompt_dead_time = get_curr_time();
      prompt_msg_open = false;
      if(msgBox.clickedButton() == yesButton)
      {
         return on_upButton_clicked();
      }
      if(msgBox.clickedButton() == noButton)
      {
         
         pthread_mutex_lock(&commutex);
         write_fifo_channel(0, "ignore\n", 7, &resp);
         pthread_mutex_unlock(&commutex);
         return;
      }
   }

   if(state_prompt == PROMPT_DOWN)
   {
      QAbstractButton *yesButton = msgBox.addButton("Yes", QMessageBox::YesRole);
      QAbstractButton *noButton = msgBox.addButton("No", QMessageBox::HelpRole);
      
      msgBox.setText("The Wollaston state is IN but you have selected a non-SDI filter.  Do you wish to set the Wollaston state to OUT?");
      prompt_msg_open = true;
      msgBox.exec();
      prompt_dead_time = get_curr_time();
      prompt_msg_open = false;
      if(msgBox.clickedButton() == yesButton)
      {
         return on_downButton_clicked();
      }
      if(msgBox.clickedButton() == noButton)
      {
         pthread_mutex_lock(&commutex);
         write_fifo_channel(0, "ignore\n", 7, &resp);
         pthread_mutex_unlock(&commutex);
         return;
      }
   }

   if(state_prompt == PROMPT_INT)
   {
      QAbstractButton *upButton = msgBox.addButton("IN", QMessageBox::YesRole);
      QAbstractButton *downButton = msgBox.addButton("OUT", QMessageBox::YesRole);
      QAbstractButton *noButton = msgBox.addButton("Ignore", QMessageBox::HelpRole); //HelpRole prevents closing the dialog
      
      msgBox.setText("The Wollaston state is INT. You should set the Wollaston state.");
      prompt_msg_open = true;
      msgBox.exec();
      prompt_dead_time = get_curr_time();
      prompt_msg_open = false;
      if(msgBox.clickedButton() == upButton)
      {
         return on_upButton_clicked();
      }
      if(msgBox.clickedButton() == downButton)
      {
         return on_downButton_clicked();
      }
      if(msgBox.clickedButton() == noButton)
      {
         pthread_mutex_lock(&commutex);
         write_fifo_channel(0, "ignore\n", 7, &resp);
         pthread_mutex_unlock(&commutex);
      }
      
      return;
   }


}

void BasicWollastonLiftForm::on_upButton_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, "up\n", 4, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicWollastonLiftForm::on_downButton_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, "down\n", 6, &resp);
   pthread_mutex_unlock(&commutex);
}


void BasicWollastonLiftForm::on_ButtonTakeLocal_clicked()
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
