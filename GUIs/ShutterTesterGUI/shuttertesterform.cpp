#include "shuttertesterform.h"

//extern fifo_list * global_fifo_list;

namespace VisAO
{
   
ShutterTesterForm::ShutterTesterForm(QWidget * Parent, Qt::WindowFlags f) : basic_ui(Parent, f)
{
   
}

ShutterTesterForm::ShutterTesterForm(int argc, char **argv, QWidget * Parent, Qt::WindowFlags f) : basic_ui(argc, argv, Parent, f)
{
   Create();
}

ShutterTesterForm::ShutterTesterForm(std::string name, const std::string &conffile, QWidget * Parent, Qt::WindowFlags f) : basic_ui(name, conffile, Parent, f)
{
   Create();
}

void ShutterTesterForm::Create()
{
   ui.setupUi(this);
   attach_basic_ui();

   ui.tabWidget->setCurrentIndex(0);
   
   ui.lineEditDuration->setFocusPolicy(Qt::ClickFocus);
   ui.lineEditFreq->setFocusPolicy(Qt::ClickFocus);
   ui.lineEditDC->setFocusPolicy(Qt::ClickFocus);
   
   ui.comboBoxTestMode->insertItem(0, "Frequency Test");
   ui.comboBoxTestMode->insertItem(1, "Simulation Replay");
   
   testing = 0;
   
   setup_fifo_list(1);
   
   std::string visao_root = getenv("VISAO_ROOT");
   std::string stfifo = (std::string)(ConfigDictionary())["ShutterTester_fifos"];
   
   
   std::string fpathin = visao_root + "/" + stfifo + "/shuttertester_com_local_in";
   std::string fpathout = visao_root + "/" + stfifo + "/shuttertester_com_local_out";
   
   signal(SIGIO, SIG_IGN);
   set_fifo_list_channel(&fl, 0, 100,fpathout.c_str(), fpathin.c_str(), 0, 0);
   
   if(connect_fifo_list() < 0)
   {
      QMessageBox msgBox;
      msgBox.setText("Another shutter tester GUI is probably already running.");
      msgBox.setInformativeText("Cannot lock the input FIFO on one or more channels.");
      msgBox.exec();
      exit(-1);
   }
   
   std::string resp;
   write_fifo_channel(0, "LOCAL\n", 6, &resp);
   
   statustimeout_normal = statustimeout;
   
   
   strehls = 0;
   n_strehls = 0;
   
   threshold = 0.;
   dutycycle = 0;
   
   return;
}

void ShutterTesterForm::attach_basic_ui()
{
   __LabelCmode = ui.LabelCmode;
   __StatusCmode = ui.StatusCmode;
   __StatusConnected = ui.StatusConnected;
   __ButtonTakeLocal = ui.ButtonTakeLocal;
   __checkBoxOverride = ui.checkBoxOverride;

   ui.cModeFrame->lower();
}

void ShutterTesterForm::retrieve_state()
{
   std::string resp;
   
   write_fifo_channel(0, "LOCAL\n", 6, &resp);
   
   if(resp == "")
   {
      return not_connected();
   }
   state_cmode = resp[0];
   
   write_fifo_channel(0, "OPEN?", 5, &resp);
   
   if(resp == "") return not_connected();
   curr_state = atoi(resp.c_str());
   
   write_fifo_channel(0, "testmode?", 9, &resp);
   
   if(resp == "") return not_connected();
   testmode = atoi(resp.c_str());
   
   write_fifo_channel(0, "duration?", 9, &resp);
   
   if(resp == "") return not_connected();
   duration = strtod(resp.c_str(),0);
   
   write_fifo_channel(0, "freq?", 5, &resp);
   
   if(resp == "") return not_connected();
   freq = strtod(resp.c_str(),0);
   
   write_fifo_channel(0, "dutycyc?\n", 9, &resp);
   
   if(resp == "") return not_connected();
   dutycyc = strtod(resp.c_str(),0);
   
   write_fifo_channel(0, "testing?\n", 10, &resp);
   
   testing = atoi(resp.c_str());
   
   
   write_fifo_channel(0, "remaining?\n", 12, &resp);
   t_remaining = strtod(resp.c_str(), 0);
   
   write_fifo_channel(0, "testfile?\n", 11, &resp);
   testfile = resp;
   
   write_fifo_channel(0, "deltat?\n", 9, &resp);
   deltat = strtod(resp.c_str(), 0);
   
   write_fifo_channel(0, "threshold?\n", 12, &resp);
   threshold = strtod(resp.c_str(), 0);
   
   state_connected = 1;
   return;
}

void ShutterTesterForm::not_connected()
{
   state_connected = 0;
   duration = -1;
   curr_state = 0;
   freq = 0.;
   dutycyc = 0;
   testmode = 0;
   return;
}

void ShutterTesterForm::update_status()
{
   char posstr[10];
   //static int i = 0;
   
   //std::cout << "updating " << i << "\n";
   //i++;
   /*if(statustimeout != statustimeout_normal)
   {
      statustimeout = statustimeout_normal;
      statustimer.start(statustimeout_normal);
   }*/
   
   snprintf(posstr, 10, "%0.2f", duration);
   if(!ui.lineEditDuration->hasFocus())
   {
      ui.lineEditDuration->setText(posstr);
      ui.lineEditDuration->clearFocus();
   }
   
   if(curr_state == 1)
   {
      ui.labelShutterState->setText("OPEN");
      ui.pushButtonOpenShut->setText("Shut");
   }
   
   if(curr_state == -1)
   {
      ui.labelShutterState->setText("SHUT");
      ui.pushButtonOpenShut->setText("Open");
   }
   
   if(curr_state == 0)
   {
      ui.labelShutterState->setText("?");
      ui.pushButtonOpenShut->setText("?");
   }
   
   if(testmode == 0) ui.comboBoxTestMode->setCurrentIndex(0);
   if(testmode == 1) ui.comboBoxTestMode->setCurrentIndex(1);
                                   
                                   
   snprintf(posstr, 10, "%0.2f", freq);
   if(!ui.lineEditFreq->hasFocus())
   {
      ui.lineEditFreq->setText(posstr);
      ui.lineEditFreq->clearFocus();
   }
   
   snprintf(posstr, 10, "%0.3f", dutycyc);
   if(!ui.lineEditDC->hasFocus())
   {
      ui.lineEditDC->setText(posstr);
      ui.lineEditDC->clearFocus();
   }
   
   snprintf(posstr, 10, "%0.4f", deltat);
   if(!ui.deltaTEntry->hasFocus())
   {
      ui.deltaTEntry->setText(posstr);
      ui.deltaTEntry->clearFocus();
   }
   
   snprintf(posstr, 10, "%0.3f", threshold);
   if(!ui.thresholdEntry->hasFocus())
   {
      ui.thresholdEntry->setText(posstr);
      ui.thresholdEntry->clearFocus();
   }
   
   if(!ui.simFileEdit->hasFocus())
   {
      ui.simFileEdit->setText(testfile.c_str());
      if(testfile != old_testfile)
      {
         update_simstats();
         old_testfile = testfile;
      }
      ui.simFileEdit->clearFocus();
   }
   
   if(state_connected == 0 || state_cmode != 'L')
   {
      ui.pushButtonStart->setEnabled(false);
      ui.pushButtonStop->setEnabled(false);
      ui.pushButtonOpenShut->setEnabled(false);
      return;
   }
   
   ui.pushButtonOpenShut->setEnabled(true);
   
   ui.pushButtonStart->setEnabled(true);
   ui.pushButtonStop->setEnabled(true);
   
   if(testing)
   {
      ui.progressBar->setVisible(true);
      ui.progressBar->setValue(t_remaining);
      //snprintf(posstr, 10, "%0.1f s", t_remaining);
      //ui.progressBar->setTe(posstr);
   }
   else ui.progressBar->setVisible(false);
                                   
}

void ShutterTesterForm::on_pushButtonStart_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, "start\n", 6, &resp);
   pthread_mutex_unlock(&commutex);
   
   ui.progressBar->setVisible(true);
   ui.progressBar->setMaximum(duration);
   ui.progressBar->setValue(duration);

   
   
}

void ShutterTesterForm::on_pushButtonStop_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, "stop\n", 5, &resp);
   pthread_mutex_unlock(&commutex);
}

void ShutterTesterForm::on_pushButtonOpenShut_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   
   if(curr_state == 1)
   {
      write_fifo_channel(0, "CLOSE", 5, &resp);
   }
   if(curr_state == -1)
   {
      write_fifo_channel(0, "OPEN", 4, &resp);
   }

   pthread_mutex_unlock(&commutex);
   
   statustimerout();
}


void ShutterTesterForm::on_lineEditDuration_editingFinished()
{
   char dstr[20];
   std::string resp;

   
   
   duration = ui.lineEditDuration->text().toDouble();
   ui.lineEditDuration->clearFocus();
   snprintf(dstr, 20, "duration %0.4f\n", duration);

   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, dstr, strlen(dstr), &resp);

   pthread_mutex_unlock(&commutex);
}

void ShutterTesterForm::on_lineEditFreq_editingFinished()
{
   char fstr[20];
   std::string resp;

  
   
   freq = ui.lineEditFreq->text().toDouble();
   ui.lineEditFreq->clearFocus();
   snprintf(fstr, 20, "freq %0.4f\n", freq);

   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, fstr, strlen(fstr), &resp);
   pthread_mutex_unlock(&commutex);
}

void ShutterTesterForm::on_lineEditDC_editingFinished()
{
   char dstr[20];
   std::string resp;

   
   
   dutycyc = ui.lineEditDC->text().toDouble();
   ui.lineEditDC->clearFocus();
   snprintf(dstr, 20, "dutycyc %0.4f\n", dutycyc);

   pthread_mutex_lock(&commutex);
   write_fifo_channel(0, dstr, strlen(dstr), &resp);
   pthread_mutex_unlock(&commutex);
}

void ShutterTesterForm::on_ButtonTakeLocal_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   
   if(ui.checkBoxOverride->isChecked())
   {
      write_fifo_channel(0, "XLOCAL\n", 7, &resp);
   }
   else
   {
      write_fifo_channel(0, "LOCAL\n", 6, &resp);
   }

   pthread_mutex_unlock(&commutex);
}

void ShutterTesterForm::update_simstats()
{
   char tmpstr[10];
   return;
   std::cout << "Here we go . . .\n";
   if(strehls) delete[] strehls;
                                   
   std::cout << testfile << "\n";
   n_strehls = readcolumns(testfile.c_str(), " ,\t", '#', "d", &strehls);
   std::cout << "n_strehls " << n_strehls << "\n";
   
   minStrehl = strehls[0];
   maxStrehl = strehls[0];
   dutycycle = 0;
   for(int i=1; i< n_strehls; i++)
   {
      if(strehls[i] < minStrehl) minStrehl = strehls[i];
                                   if(strehls[i] > maxStrehl) maxStrehl = strehls[i];
                                   if(strehls[i] >= threshold) dutycycle++;
   }
   
   std::cout << minStrehl << "\n";
   snprintf(tmpstr, 10, "%0.2f", minStrehl);
   ui.minStrehl->setText(tmpstr);
   
   snprintf(tmpstr, 10, "%0.2f", maxStrehl);
   ui.maxStrehl->setText(tmpstr);
   
   dutycycle /= n_strehls;
   snprintf(tmpstr, 10, "%0.2f", dutycycle);
   ui.simDutyCycle->setText(tmpstr);
}

void ShutterTesterForm::on_simFileSelect_clicked()
{
   
   QString newfile = QFileDialog::getOpenFileName(0, "Select Simulation File", "$VISAO_ROOT/bin/sims/data", "");
   
   testfile = newfile.toStdString();
   
   ui.simFileEdit->setText(newfile);
   
   //std::cout << newfile.toStdString() << "\n";
   
   update_simstats();
   
   
}

void ShutterTesterForm::on_thresholdEntry_editingFinished()
{
   char tmpstr[10];
   threshold = ui.thresholdEntry->text().toDouble();
   dutycycle = 0;
   for(int i=1; i< n_strehls; i++)
   {
      if(strehls[i] >= threshold) dutycycle++;
   }
   
   dutycycle /= n_strehls;
   snprintf(tmpstr, 10, "%0.2f", dutycycle);
   ui.simDutyCycle->setText(tmpstr);
}

void ShutterTesterForm::on_comboBoxTestMode_activated(int tmode)
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   
   if(tmode == 0)
   {
      write_fifo_channel(0, "testmode 0\n", 11, &resp);
      pthread_mutex_unlock(&commutex);
      return;
   }
   
   if(tmode == 1)
   {
      write_fifo_channel(0, "testmode 1\n", 11, &resp);
      pthread_mutex_unlock(&commutex);
      return;
   }

   
   
}

} //namespace VisAO
