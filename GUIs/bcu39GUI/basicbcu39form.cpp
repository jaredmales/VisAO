#include "basicbcu39form.h"

#include <iostream>

namespace VisAO
{

BasicBCU39Form::BasicBCU39Form(QWidget * Parent, Qt::WindowFlags f) : basic_ui(Parent, f)
{

}

BasicBCU39Form::BasicBCU39Form(int argc, char **argv, QWidget * Parent, Qt::WindowFlags f) : basic_ui(argc, argv, Parent, f)
{
   Create();
}

BasicBCU39Form::BasicBCU39Form(std::string name, const std::string& conffile, QWidget * Parent, Qt::WindowFlags f) : basic_ui(name, conffile, Parent, f)
{
   Create();
}

#define FG39_FIFO_CH  0
#define FW39_FIFO_CH  1
#define RECON_FIFO_CH 2

void BasicBCU39Form::Create()
{
   ui.setupUi(this);
   attach_basic_ui();
   
   setup_fifo_list(3);
   
   std::string visao_root = getenv("VISAO_ROOT");
   std::string fg39_fifopath;

   try
   {
      fg39_fifopath = (std::string)(ConfigDictionary())["fg39_fifopath"];
   }
   catch(...)
   {
      fg39_fifopath = "fifos";
   }

   #ifdef VISAO_SIMDEV
   std::string fpathin = visao_root + "/" + fg39_fifopath + "/framegrabber39_sim_com_local_in";
   std::string fpathout = visao_root + "/" + fg39_fifopath + "/framegrabber39_sim_com_local_out";
   #elif defined VISAO_SIM
   std::string fpathin = visao_root + "/" + fg39_fifopath + "/framegrabber39_sim_com_local_in";
   std::string fpathout = visao_root + "/" + fg39_fifopath + "/framegrabber39_sim_com_local_out";
   #else
   std::string fpathin = visao_root + "/" + fg39_fifopath + "/framegrabber39_com_local_in";
   std::string fpathout = visao_root + "/" + fg39_fifopath + "/framegrabber39_com_local_out";
   #endif

   signal(SIGIO, SIG_IGN);
   set_fifo_list_channel(&fl, FG39_FIFO_CH, 100,fpathout.c_str(), fpathin.c_str(), 0, 0);

   //******** FW 39 FIFO **************
   fpathin = visao_root + "/" + fg39_fifopath + "/framewriter39_com_auto_in";
   fpathout = visao_root + "/" + fg39_fifopath + "/framewriter39_com_auto_out";

   set_fifo_list_channel(&fl, FW39_FIFO_CH, 100,fpathout.c_str(), fpathin.c_str(), 0, 0);

   //******** Reconstructor FIFO ***********
   fpathin = visao_root + "/" + fg39_fifopath + "/reconstructor_com_local_in";
   fpathout = visao_root + "/" + fg39_fifopath + "/reconstructor_com_local_out";

   set_fifo_list_channel(&fl, RECON_FIFO_CH, 100,fpathout.c_str(), fpathin.c_str(), 0, 0);

   
   if(connect_fifo_list() < 0)
   {
      QMessageBox msgBox;
      msgBox.setText("Another BCU 39 Telemetry GUI is probably already running.");
      msgBox.setInformativeText("Cannot lock the input FIFO on one or more channels.");
      msgBox.exec();
      exit(-1);
   }

   set_wait_to(1.);

   fg39sb = 0;
   fw39sb = 0;
   rsb = 0;
   
   strehlPlot = 0;
   wfePlot = 0;
   ttPlot = 0;
   ho1Plot = 0;
   ho2Plot = 0;
   
   if(strehl_sis.attach_shm(5003) < 0)
   {
      ERROR_REPORT("Error attaching to shared memory for strehls.");
      strehlAttached = false;
   }
   else strehlAttached = true;

   strehlPlotUpdateInt = 250.;
   connect(&strehlPlotTimer, SIGNAL(timeout()), this, SLOT(updateStrehlPlot()));
   strehlPlotTimer.start(strehlPlotUpdateInt);
   
   statustimerout();

   setLambda(0.765);
   applyFitError = 0;
   
   ui.lambdaEdit->setText("0.765");

   ui.lambdaBox->insertItem(0,  "                ");
   ui.lambdaBox->insertItem(1,  "  r'  (0.624 um)");
   ui.lambdaBox->insertItem(2,  "[O I] (0.630 um)");
   ui.lambdaBox->insertItem(3,  "  Ha  (0.656 um)");
   ui.lambdaBox->insertItem(4,  "[S II](0.672 um)");
   ui.lambdaBox->insertItem(5,  "  i'  (0.765 um)");
   ui.lambdaBox->insertItem(6,  "  z'  (0.906 um)");
   ui.lambdaBox->insertItem(7,  "  Ys  (0.982 um)");
   ui.lambdaBox->insertItem(8,  "  J   (1.25 um)");
   ui.lambdaBox->insertItem(9,  "  H   (1.64 um)");
   ui.lambdaBox->insertItem(10, "  Ks  (2.15 um)");
   ui.lambdaBox->insertItem(11, " 3.1  (3.10 um)");
   ui.lambdaBox->insertItem(12, " 3.3  (3.32 um)");
   ui.lambdaBox->insertItem(13, "  L'  (3.77 um)");
   ui.lambdaBox->insertItem(14, "  M'  (4.69 um)");

   ui.lambdaBox->setCurrentIndex(5);

   ui.lambdaBox->setLayoutDirection(Qt::LeftToRight);
   
   ui.rawCheck->setCheckState(Qt::Checked);
   ui.filteredCheck->setCheckState(Qt::Unchecked);
   plotRawStrehl = 1;
   plotFilteredStrehl = 0;
   plotRMS = 0;

   QString pathbase = getenv("VISAO_ROOT");
   pathbase += "/calib/visao/reconstructor";

   recPath = pathbase + "/RecMats/";
   filterPath = pathbase + "/filters/";
   
   ui.fg39fps->setStyleSheet(paramStyle);
   ui.wfe1avg->setStyleSheet(paramStyle);
   ui.wfe1std->setStyleSheet(paramStyle);
   //ui.wfeInst->setStyleSheet(paramStyle);
   
   ui.recMatFile->setStyleSheet(paramStyle);
   return;
}

void BasicBCU39Form::attach_basic_ui()
{
   __LabelCmode = ui.LabelCmode;
   __StatusCmode = ui.StatusCmode;
   __StatusConnected = ui.StatusConnected;
   __ButtonTakeLocal = ui.ButtonTakeLocal;
   __checkBoxOverride = ui.checkBoxOverride;

   ui.cModeFrame->lower();
}

void BasicBCU39Form::retrieve_state()
{
   //double startT;
   std::string resp;
   
   write_fifo_channel(FG39_FIFO_CH, "state?\n", 8, &resp);
   
   //startT = get_curr_time();
   if(resp == "" || resp.length()<5)
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
      std::cerr << "Error parsing fg39stat\n";
      state_connected = 0;
      state_cmode = -1;
      return;
   }
   
   fg39stat = atoi(resp.substr(st+1, sp-st-1).c_str());

   st = resp.find(',',sp);
   sp = resp.find(',', st+1);
   
   if(st < 0 || sp < 0)
   {
      std::cerr << "Error parsing saving\n";
      state_connected = 0;
      state_cmode = -1;
      return;
   }
   saving = atoi(resp.substr(st+1, sp-st-1).c_str());

   st = resp.find(',',sp);
   sp = resp.find(',', st+1);
   
   if(st < 0 || sp < 0)
   {
      std::cerr << "Error parsing skipping\n";
      state_connected = 0;
      state_cmode = -1;
      return;
   }
   
   skipping = atoi(resp.substr(st+1, sp-st-1).c_str());

   st = resp.find(',',sp);
   sp = resp.find(',', st+1);
   
   if(st < 0 || sp < 0)
   {
      std::cerr << "Error parsing remaining\n";
      state_connected = 0;
      state_cmode = -1;
      return;
   }
   
   remaining = atoi(resp.substr(st+1, sp-st-1).c_str());

   st = resp.find(',',sp);
   //sp = resp.find(',', st+1);
   
   if(st < 0)
   {
      std::cerr << "Error parsing reconstat\n";
      state_connected = 0;
      state_cmode = -1;
      return;
   }
   
   reconstat = atoi(resp.substr(st+1, resp.length()-st-1).c_str());

   write_fifo_channel(RECON_FIFO_CH, "recmat?\n", 8, &fullRecPath);
   write_fifo_channel(RECON_FIFO_CH, "filter?\n", 8, &fullFilterPath);
   write_fifo_channel(RECON_FIFO_CH, "lambda?\n", 8, &resp);
   lambda = strtod(resp.c_str(),0)/1000.;

   write_fifo_channel(RECON_FIFO_CH, "cal_a?\n", 7, &resp);
   cal_a = strtod(resp.c_str(),0);

   write_fifo_channel(RECON_FIFO_CH, "cal_b?\n", 7, &resp);
   cal_b = strtod(resp.c_str(),0);
   
   return;
   
}
void BasicBCU39Form::update_status()
{
   size_t sz;
   char tmp[256];
   QString styleUp = "background-color : lime; color : black; ";
   QString styleDown = "background-color : red; color : black; ";
   QString styleCaution = "background-color : yellow; color : black; ";
   
   if(!state_connected)
   {
      ui.fg39Status->setText("unkown");
      updateFW39Status();
      ui.reconStatus->setText("unknown");
      return;
   }

   if(!fg39sb) fg39sb = (framegrabber39_status_board*) attach_shm(&sz,  STATUS_framegrabber39, 0);
   
   if(fg39sb)
   {
      /*if((get_curr_time() - ts_to_curr_time(&fw47sb->last_update)) < fw47sb->max_update_interval)
       {                                         *
       ui.fw47Status->setStyleSheet(upStyle);
       }
       else ui.fw47Status->setStyleSheet(downStyle);*/
   }
   //else ui.fw47Status->setStyleSheet(downStyle);
   
   if(fg39stat)
   {
      if(fg39sb) snprintf(tmp, 256, "%0.3f fps", fg39sb->fps);
      else tmp[0] = '\0';
      ui.fg39fps->setText(tmp);
      ui.fg39Status->setText("running");
      ui.startButton->setText("Stop FG39");
   }
   else
   {
      ui.fg39fps->setText("");
      ui.fg39Status->setText("stopped");
      ui.startButton->setText("Start FG39");
   }

   if(reconstat)
   {
      if(fg39stat)
      {
         ui.reconStatus->setText("running . . .");
      }
      else
      {
         ui.reconStatus->setText("standing by");
      }
      ui.reconButton->setText("Stop Reconstructing");
   }
   else
   {
      ui.reconStatus->setText("stopped.");
      ui.reconButton->setText("Start Reconstructing");
   }

   if(!rsb) rsb = (reconstructor_status_board*) attach_shm(&sz,  STATUS_reconstructor, 0);
   
   if(rsb)
   {
      snprintf(tmp, 256, "%0.1f", rsb->avgwfe_1_sec);
      ui.wfe1avg->setText(tmp);
            
      snprintf(tmp, 256, "%0.1f", rsb->stdwfe_1_sec);
      ui.wfe1std->setText(tmp);
      
      //snprintf(tmp, 256, "%0.1f", rsb->inst_wfe);
      //ui.wfeInst->setText(tmp);
   }
   
   if(saving) 
   {
      ui.saveButton->setText("Stop Saving");
      ui.saveDirButton->setEnabled(false);
      ui.saveDirInput->setEnabled(false);
      ui.labelSave->setEnabled(false);
      ui.saveFrames->setEnabled(false);
      ui.labelFrames->setEnabled(false);
      ui.saveCont->setEnabled(false);
      ui.labelSkip->setEnabled(false);
      ui.skipFrames->setEnabled(false);
      ui.labelFrames1->setEnabled(false);
   }
   else 
   {
      ui.saveButton->setText("Start Saving");
      ui.saveDirButton->setEnabled(true);
      ui.saveDirInput->setEnabled(true);
      if(ui.saveCont->checkState() == Qt::Checked)
      {
         ui.labelSave->setEnabled(false);
         ui.saveFrames->setEnabled(false);
         ui.labelFrames->setEnabled(false);
      }
      else
      {
         ui.labelSave->setEnabled(true);
         ui.saveFrames->setEnabled(true);
         ui.labelFrames->setEnabled(true);
      }
      ui.saveCont->setEnabled(true);
      ui.labelSkip->setEnabled(true);
      ui.skipFrames->setEnabled(true);
      ui.labelFrames1->setEnabled(true);
   }

   updateFW39Status();

   updateRecStatus();
   
}

void BasicBCU39Form::updateFW39Status()
{
   size_t sz;
   QString styleUp = "";
   QString styleDown = "background-color : red; color : black; ";
   QString styleCaution = "background-color : yellow; color : black; ";
   
   if(!fw39sb) fw39sb = (basic_status_board*) attach_shm(&sz,  STATUS_framewriter39, 0);
   
   if(fw39sb)
   {
      double t = get_curr_time();
      double dt = ts_to_curr_time(&fw39sb->last_update);
      if(t - dt > fw39sb->max_update_interval && t-dt < 3*fw39sb->max_update_interval)
      {
         ui.fw39status->setStyleSheet(styleCaution);
      }
      else if(t-dt >= 3*fw39sb->max_update_interval)
      {
         ui.fw39status->setStyleSheet(styleDown);
      }
      else
      {
         ui.fw39status->setStyleSheet(styleUp);
      }
   }
   else
   {
      ui.fw39status->setStyleSheet(styleDown);
   }
}

void BasicBCU39Form::updateRecStatus()
{
   if(!ui.recMatFile->hasFocus())
   {
      ui.recMatFile->setText(fullRecPath.c_str());
   }

   if(!ui.filterFile->hasFocus())
   {
      ui.filterFile->setText(fullFilterPath.c_str());
   }

   if(!ui.lambdaEdit->hasFocus())
   {
      ui.lambdaEdit->setText(QString::number(lambda, 'f', 3));
   }

   if(!ui.calA->hasFocus())
   {
      ui.calA->setText(QString::number(cal_a, 'f', 3));
   }

   if(!ui.calB->hasFocus())
   {
      ui.calB->setText(QString::number(cal_b, 'f', 3));
   }
   
}

   
void BasicBCU39Form::setLambda(double newlam)
{
   std::string com, resp;
   QString lamstr;
   
   lambda = newlam;

   std::cout << lambda << "\n";
   
   lamstr = QString::number(lambda*1000., 'f', 6);
   
   com = "lambda ";
   com += lamstr.toStdString();
   com += "\n";
   
   std::cout << com << "\n";
   pthread_mutex_lock(&commutex);
   write_fifo_channel(RECON_FIFO_CH, com.c_str(), com.length()+1, &resp);
   pthread_mutex_unlock(&commutex);

   if(strehlPlot)
   {
      strehlPlot->reset();
   }
}

void BasicBCU39Form::on_startButton_clicked()
{
   std::string resp;
   
   pthread_mutex_lock(&commutex);
   
   if(fg39stat == 0)
      write_fifo_channel(FG39_FIFO_CH, "start\n", 7, &resp);
   
   if(fg39stat == 1)
      write_fifo_channel(FG39_FIFO_CH, "stop\n", 6, &resp);
   
   pthread_mutex_unlock(&commutex);
}

void BasicBCU39Form::on_saveDirButton_clicked()
{
   QString dirName = QFileDialog::getExistingDirectory(this, tr("Save to directory"), "/home/aosup/visao/data/ccd39");
   
   if(!dirName.startsWith("/home/aosup/visao/data/ccd39"))
   {
      QMessageBox msgBox;
      msgBox.setText("Invalid CCD39 Data Directory");
      msgBox.setInformativeText("CCD39 data can only be saved in ~/visao/data/ccd39/ and subdirectories.");
      msgBox.exec();
      return;
   }
   
   dirName.remove("/home/aosup/visao/data/ccd39");
   
   if(dirName[0] == '/') dirName.remove(0,1);
            
   ui.saveDirInput->setText(dirName);
   
}

void BasicBCU39Form::on_saveCont_stateChanged(int state)
{
   if(state == Qt::Checked)
   {
      ui.labelSave->setEnabled(false);
      ui.labelFrames->setEnabled(false);
      ui.saveFrames->setEnabled(false);
   }
   else
   {
      ui.labelSave->setEnabled(true);
      ui.labelFrames->setEnabled(true);
      ui.saveFrames->setEnabled(true);
   }  
}   
   
void BasicBCU39Form::on_saveButton_clicked()
{
   int nskip, nsave;
   char tmp[256];
   std::string resp;

   if(!saving)
   {
      std::string subdir;
      subdir = ui.saveDirInput->text().toStdString();
      snprintf(tmp, 256, "subdir %s\n", subdir.c_str());
      std::cout << tmp << "\n";
      write_fifo_channel(FW39_FIFO_CH, "AUTO", 4, &resp);
      std::cout << resp << "\n";
      
      write_fifo_channel(FW39_FIFO_CH, tmp, strlen(tmp), &resp);
      std::cout << resp << "\n";
      
      nskip = ui.skipFrames->text().toInt();
      if(nskip >= 0)
      {
         snprintf(tmp, 25, "skip %i\n", nskip);
         write_fifo_channel(FG39_FIFO_CH, tmp, strlen(tmp)+1, &resp);
      }
      else write_fifo_channel(FG39_FIFO_CH, "skip 0\n", 8, &resp);
   
      if(ui.saveCont->isChecked())
      {
         nsave = -1;
      }
      else
      {
         nsave = ui.saveFrames->text().toInt();
         if(nsave < 0) nsave = 0;
      }

      snprintf(tmp, 25, "save %i\n", nsave);
      write_fifo_channel(FG39_FIFO_CH, tmp, strlen(tmp)+1, &resp);
      ui.saveButton->setText("Stop Saving");
      
      ui.saveDirButton->setEnabled(false);
      ui.saveDirInput->setEnabled(false);
      ui.labelSave->setEnabled(false);
      ui.saveFrames->setEnabled(false);
      ui.labelFrames->setEnabled(false);
      ui.saveCont->setEnabled(false);
      ui.labelSkip->setEnabled(false);
      ui.skipFrames->setEnabled(false);
      ui.labelFrames1->setEnabled(false);
      
   }
   else
   {
      write_fifo_channel(FG39_FIFO_CH, "save 0\n", 8, &resp);
      ui.saveButton->setText("Start Saving");
      
      ui.saveDirButton->setEnabled(true);
      ui.saveDirInput->setEnabled(true);
      if(ui.saveCont->checkState() == Qt::Checked)
      {
         ui.labelSave->setEnabled(false);
         ui.saveFrames->setEnabled(false);
         ui.labelFrames->setEnabled(false);
      }
      else
      {
         ui.labelSave->setEnabled(true);
         ui.saveFrames->setEnabled(true);
         ui.labelFrames->setEnabled(true);
      }
      ui.saveCont->setEnabled(true);
      ui.labelSkip->setEnabled(true);
      ui.skipFrames->setEnabled(true);
      ui.labelFrames1->setEnabled(true);
      
   }
}

void BasicBCU39Form::on_recFileButton_clicked()
{
   QString fileName;
   std::string resp, com;
   
   
   fileName = QFileDialog::getOpenFileName(this, "Select Reconstructor Matrix",
                                           recPath, "*.fits (*.fits)");

   if(fileName == "") return;
   
   com = "recmat ";
   com += fileName.toStdString();
   com += "\n";

   std::cout << com << "\n";
   pthread_mutex_lock(&commutex);

  
   write_fifo_channel(RECON_FIFO_CH, com.c_str(), com.length()+1, &resp);
  
   std::cout << resp << "\n";
   pthread_mutex_unlock(&commutex);

   ui.recMatFile->setText(fileName);
}

void BasicBCU39Form::on_lambdaEdit_editingFinished()
{
   QString qstr = ui.lambdaEdit->text();

   double newlam = qstr.toDouble();

   ui.lambdaBox->setCurrentIndex(0);
   
   setLambda(newlam);
}

void BasicBCU39Form::on_lambdaBox_activated(int index)
{
   float lam;
   char lamstr[10];
   switch(index)
   {
      case 0:
         break;
      case 1:
         lam = 0.624;
         break;
      case 2:
         lam = 0.630;
         break;
      case 3:
         lam = 0.656;
         break;
      case 4:
         lam = 0.672;
         break;
      case 5:
         lam = 0.765;
         break;
      case 6:
         lam = 0.906;
         break;
      case 7:
         lam = 0.982;
         break;
      case 8:
         lam = 1.25;
         break;
      case 9:
         lam = 1.64;
         break;
      case 10:
         lam = 2.15;
         break;
      case 11:
         lam = 3.10;
         break;
      case 12:
         lam = 3.32;
         break;
      case 13:
         lam = 3.77;
         break;
      case 14:
         lam = 4.69;
         break;
      default:
         break;
   }
   setLambda(lam);
   snprintf(lamstr, 10, "%0.3f", lam);
   ui.lambdaEdit->setText(lamstr);
}

void BasicBCU39Form::on_filterFileButton_clicked()
{
   QString fileName;
   std::string resp, com;
   
   fileName = QFileDialog::getOpenFileName(this, "Select Filter Coefficients",
                                           filterPath, "*.txt (*.txt)");
   
   if(fileName == "") return;
   
   com = "filter ";
   com += fileName.toStdString();
   com += "\n";
   
   //std::cout << com << "\n";
   pthread_mutex_lock(&commutex);
   write_fifo_channel(RECON_FIFO_CH, com.c_str(), com.length()+1, &resp);
   pthread_mutex_unlock(&commutex);
   
   ui.filterFile->setText(fileName);
}

void BasicBCU39Form::on_calA_editingFinished()
{
   std::string resp;

   QString qstr = ui.calA->text();

   std::string com = "cal_a ";
   com += qstr.toStdString();

   pthread_mutex_lock(&commutex);
   write_fifo_channel(RECON_FIFO_CH, com.c_str(), com.length()+1, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicBCU39Form::on_calB_editingFinished()
{
   std::string resp;

   QString qstr = ui.calB->text();

   std::string com = "cal_b ";
   com += qstr.toStdString();

   pthread_mutex_lock(&commutex);
   write_fifo_channel(RECON_FIFO_CH, com.c_str(), com.length()+1, &resp);
   pthread_mutex_unlock(&commutex);
}

void BasicBCU39Form::on_reconButton_clicked()
{
   std::string resp;
   
   pthread_mutex_lock(&commutex);
   
   if(reconstat == 0)
      write_fifo_channel(FG39_FIFO_CH, "serve 1\n", 8, &resp);
   
   if(reconstat == 1)
      write_fifo_channel(FG39_FIFO_CH, "serve 0\n", 8, &resp);
   
   pthread_mutex_unlock(&commutex);
}









void BasicBCU39Form::on_plotButton_clicked()
{
   if(!strehlPlot)
   {
      if(!strehlAttached)
      {
         if(strehl_sis.attach_shm(5003) < 0)
         {
            ERROR_REPORT("Error attaching to shared memory for strehls.");
            strehlAttached = false;
            return;
         }
         else strehlAttached = true;
      }
      last_strehl = strehl_sis.get_last_image();
      if(last_strehl < 0) return;
      
      strehlPlot = new rtPlotForm("Strehl Ratio", "SR", 5000, 1, statustimeout, this);
      //connect(fwhmPlot, SIGNAL(rejected()), this, SLOT(fwhmPlotClosed()));

      
      curr_strehl = last_strehl - 5000;
      if(curr_strehl < 0)
      {
         curr_strehl += strehl_sis.get_n_images();
         //Check to see if there are enough images after last_strehl to continue
         //this only matters are startup, and can almost never happen due to high data rate of strehls
         if(curr_strehl <= last_strehl) curr_strehl = 0;
      }
      for(int i =0; i< 5000; i++)
      {
         strehl_sim = strehl_sis.get_image(curr_strehl);

         float sr;

         if(plotRawStrehl)
         {
            if(applyFitError) sr = strehl_sim.imdata[4];
            else sr = strehl_sim.imdata[5];
         }
         else
         {
            if(applyFitError) sr = strehl_sim.imdata[6];
            else sr = strehl_sim.imdata[7];
         }
         
         strehlPlot->data->add_point(tv_to_curr_time(&strehl_sim.frame_time), sr);
         
         curr_strehl++;
         if(curr_strehl > strehl_sis.get_n_images() - 1)
         {
            curr_strehl = 0;
         }
         if(curr_strehl == last_strehl) break;
      }
   }
   strehlPlot->show();
}

void BasicBCU39Form::on_plotWFEButton_clicked()
{
   if(!wfePlot)
   {
      if(!strehlAttached)
      {
         if(strehl_sis.attach_shm(5003) < 0)
         {
            ERROR_REPORT("Error attaching to shared memory for strehls.");
            strehlAttached = false;
            return;
         }
         else strehlAttached = true;
      }
      last_strehl = strehl_sis.get_last_image();
      if(last_strehl < 0) return;
      
      wfePlot = new rtPlotForm("Wavefront Error", "WFE (nm rms)", 5000, 1, statustimeout, this);
      //connect(fwhmPlot, SIGNAL(rejected()), this, SLOT(fwhmPlotClosed()));
      
      
      curr_strehl = last_strehl - 5000;
      if(curr_strehl < 0)
      {
         curr_strehl += strehl_sis.get_n_images();
         //Check to see if there are enough images after last_strehl to continue
         //this only matters are startup, and can almost never happen due to high data rate of strehls
         if(curr_strehl <= last_strehl) curr_strehl = 0;
      }
      for(int i =0; i< 5000; i++)
      {
         strehl_sim = strehl_sis.get_image(curr_strehl);
         
         float wfe;
         
         if(plotRawStrehl)
         {
            wfe = sqrt(strehl_sim.imdata[0]);
         }
         else if(plotFilteredStrehl)
         {
            wfe = strehl_sim.imdata[1];
         }
         else
         {
            wfe = strehl_sim.imdata[14];
         }
         
         if(wfe < 1000 && wfe > 0)
         wfePlot->data->add_point(tv_to_curr_time(&strehl_sim.frame_time), wfe);
         
         curr_strehl++;
         if(curr_strehl > strehl_sis.get_n_images() - 1)
         {
            curr_strehl = 0;
         }
         if(curr_strehl == last_strehl) break;
      }
   }
   wfePlot->show();
}


void BasicBCU39Form::on_plotTTWFEButton_clicked()
{
   if(!ttPlot)
   {
      if(!strehlAttached)
      {
         if(strehl_sis.attach_shm(5003) < 0)
         {
            ERROR_REPORT("Error attaching to shared memory for strehls.");
            strehlAttached = false;
            return;
         }
         else strehlAttached = true;
      }
      last_strehl = strehl_sis.get_last_image();
      if(last_strehl < 0) return;
      
      ttPlot = new rtPlotForm("T/T Wavefront Error", "WFE (nm rms)", 5000, 1, statustimeout, this);
      //connect(fwhmPlot, SIGNAL(rejected()), this, SLOT(fwhmPlotClosed()));
      
      
      curr_strehl = last_strehl - 5000;
      if(curr_strehl < 0)
      {
         curr_strehl += strehl_sis.get_n_images();
         //Check to see if there are enough images after last_strehl to continue
         //this only matters are startup, and can almost never happen due to high data rate of strehls
         if(curr_strehl <= last_strehl) curr_strehl = 0;
      }
      for(int i =0; i< 5000; i++)
      {
         strehl_sim = strehl_sis.get_image(curr_strehl);
         
         float wfe;
         
         if(plotRawStrehl)
         {
            wfe = sqrt(strehl_sim.imdata[2]*strehl_sim.imdata[2] + strehl_sim.imdata[3]*strehl_sim.imdata[3]);
         }
         else if(plotFilteredStrehl)
         {
            wfe = sqrt(strehl_sim.imdata[2]*strehl_sim.imdata[2] + strehl_sim.imdata[3]*strehl_sim.imdata[3]);
         }
         else
         {
            wfe = sqrt(strehl_sim.imdata[15]+strehl_sim.imdata[16])*2.*1e9;
         }
         
         if(wfe < 1000 && wfe > 0)
            ttPlot->data->add_point(tv_to_curr_time(&strehl_sim.frame_time), wfe);
         
         curr_strehl++;
         if(curr_strehl > strehl_sis.get_n_images() - 1)
         {
            curr_strehl = 0;
         }
         if(curr_strehl == last_strehl) break;
      }
   }
   ttPlot->show();
}


void BasicBCU39Form::on_plotHO1WFEButton_clicked()
{
   if(!ho1Plot)
   {
      if(!strehlAttached)
      {
         if(strehl_sis.attach_shm(5003) < 0)
         {
            ERROR_REPORT("Error attaching to shared memory for strehls.");
            strehlAttached = false;
            return;
         }
         else strehlAttached = true;
      }
      last_strehl = strehl_sis.get_last_image();
      if(last_strehl < 0) return;
      
      ho1Plot = new rtPlotForm("HO1 Wavefront Error", "WFE (nm rms)", 5000, 1, statustimeout, this);
      //connect(fwhmPlot, SIGNAL(rejected()), this, SLOT(fwhmPlotClosed()));
      
      
      curr_strehl = last_strehl - 5000;
      if(curr_strehl < 0)
      {
         curr_strehl += strehl_sis.get_n_images();
         //Check to see if there are enough images after last_strehl to continue
         //this only matters are startup, and can almost never happen due to high data rate of strehls
         if(curr_strehl <= last_strehl) curr_strehl = 0;
      }
      for(int i =0; i< 5000; i++)
      {
         strehl_sim = strehl_sis.get_image(curr_strehl);
         
         float wfe;
         
         if(plotRawStrehl)
         {
            //std::cout << strehl_sim.imdata[10] << "\n";
            wfe = sqrt(strehl_sim.imdata[10]);
         }
         else if(plotFilteredStrehl)
         {
            wfe = sqrt(strehl_sim.imdata[10]);
         }
         else
         {
            wfe = sqrt(strehl_sim.imdata[17]);
         }
           
         if(wfe < 1000 && wfe > 0)
            ho1Plot->data->add_point(tv_to_curr_time(&strehl_sim.frame_time), wfe);
         
         curr_strehl++;
         if(curr_strehl > strehl_sis.get_n_images() - 1)
         {
            curr_strehl = 0;
         }
         if(curr_strehl == last_strehl) break;
      }
   }
   ho1Plot->show();
}


void BasicBCU39Form::on_plotHO2WFEButton_clicked()
{
   if(!ho2Plot)
   {
      if(!strehlAttached)
      {
         if(strehl_sis.attach_shm(5003) < 0)
         {
            ERROR_REPORT("Error attaching to shared memory for strehls.");
            strehlAttached = false;
            return;
         }
         else strehlAttached = true;
      }
      last_strehl = strehl_sis.get_last_image();
      if(last_strehl < 0) return;
      
      ho2Plot = new rtPlotForm("HO2 Wavefront Error", "WFE (nm rms)", 5000, 1, statustimeout, this);
      //connect(fwhmPlot, SIGNAL(rejected()), this, SLOT(fwhmPlotClosed()));
      
      
      curr_strehl = last_strehl - 5000;
      if(curr_strehl < 0)
      {
         curr_strehl += strehl_sis.get_n_images();
         //Check to see if there are enough images after last_strehl to continue
         //this only matters are startup, and can almost never happen due to high data rate of strehls
         if(curr_strehl <= last_strehl) curr_strehl = 0;
      }
      for(int i =0; i< 5000; i++)
      {
         strehl_sim = strehl_sis.get_image(curr_strehl);
         
         float wfe;
         
         if(plotRawStrehl)
         {
            wfe = sqrt(strehl_sim.imdata[12]);
         }
         else if(plotFilteredStrehl)
         {
            wfe = sqrt(strehl_sim.imdata[12]);
         }
         else
         {
            wfe = sqrt(strehl_sim.imdata[18]);
         }
         
         if(wfe < 1000 && wfe > 0)
            ho2Plot->data->add_point(tv_to_curr_time(&strehl_sim.frame_time), wfe);
         
         curr_strehl++;
         if(curr_strehl > strehl_sis.get_n_images() - 1)
         {
            curr_strehl = 0;
         }
         if(curr_strehl == last_strehl) break;
      }
   }
   ho2Plot->show();
}

void BasicBCU39Form::on_ButtonTakeLocal_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   if(state_cmode == 'L')
   {
      write_fifo_channel(FG39_FIFO_CH, "~LOCAL\n", 8, &resp);
   }
   else
   {
      if(ui.checkBoxOverride->isChecked())
      {
         write_fifo_channel(FG39_FIFO_CH, "XLOCAL\n", 8, &resp);
      }
      else
      {
         write_fifo_channel(FG39_FIFO_CH, "LOCAL\n", 7, &resp);
      }
   }

   pthread_mutex_unlock(&commutex);
}



void BasicBCU39Form::on_fitCheck_stateChanged(int st)
{
   applyFitError = st;
   //std::cout << "apply fit error: " << applyFitError << "\n";
}

void BasicBCU39Form::on_rawCheck_stateChanged(int st)
{
   plotRawStrehl = st;

   if(st > 0)
   {
      plotFilteredStrehl = 0;
      plotRMS = 0;
      ui.filteredCheck->setChecked(false);
      ui.rmsCheck->setChecked(false);
   }
}

void BasicBCU39Form::on_filteredCheck_stateChanged(int st)
{
   plotFilteredStrehl = st;
   
   if(st > 0)
   {
      plotRawStrehl = 0;
      plotRMS = 0;
      ui.rawCheck->setChecked(false);
      ui.rmsCheck->setChecked(false);
   }
}

void BasicBCU39Form::on_rmsCheck_stateChanged(int st)
{
   plotRMS = st;
   
   if(st > 0)
   {
      plotRawStrehl = 0;
      plotFilteredStrehl = 0;
      ui.rawCheck->setChecked(false);
      ui.filteredCheck->setChecked(false);
   }
}


void BasicBCU39Form::updateStrehlPlot()
{
   float sr, wfe, tt, ho1, ho2;
   
   if(strehlPlot || wfePlot || ttPlot || ho1Plot || ho2Plot)
   {
      last_strehl = strehl_sis.get_last_image();
      
      int i=0;
      while(curr_strehl != last_strehl && i < 5000 )
      {
         strehl_sim = strehl_sis.get_image(curr_strehl);
         if(strehl_sim.nx <=0)
         {
            curr_strehl = 0;
            return;
         }

         if(plotRawStrehl)
         {
            if(applyFitError) sr = strehl_sim.imdata[5];
            else sr = strehl_sim.imdata[4];

            wfe = sqrt(strehl_sim.imdata[0]);
            
            tt = sqrt(strehl_sim.imdata[2]*strehl_sim.imdata[2] + strehl_sim.imdata[3]*strehl_sim.imdata[3]);
            ho1 = sqrt(strehl_sim.imdata[10]);
            ho2 = sqrt(strehl_sim.imdata[12]);
         }
         else if(plotFilteredStrehl)
         {
            if(applyFitError) sr = strehl_sim.imdata[7];
            else sr = strehl_sim.imdata[6];

            wfe = sqrt(strehl_sim.imdata[1]);
            
            tt = sqrt(strehl_sim.imdata[2]*strehl_sim.imdata[2] + strehl_sim.imdata[3]*strehl_sim.imdata[3]);
            ho1 = sqrt(strehl_sim.imdata[10]);
            ho2 = sqrt(strehl_sim.imdata[12]);
         }
         else
         {
            if(applyFitError) sr = strehl_sim.imdata[5];
            else sr = strehl_sim.imdata[4];

            wfe = sqrt(strehl_sim.imdata[14]);
            
            tt = sqrt(strehl_sim.imdata[15] + strehl_sim.imdata[16]);
            ho1 = sqrt(strehl_sim.imdata[17]);
            ho2 = sqrt(strehl_sim.imdata[18]);
         }
         
         //if(applyFitError) sigma -= fitError;
         if(strehlPlot)
         strehlPlot->data->add_point(tv_to_curr_time(&strehl_sim.frame_time), sr);
         if(wfePlot && (wfe < 1000 && wfe > 0))
            wfePlot->data->add_point(tv_to_curr_time(&strehl_sim.frame_time), wfe);
         if(ttPlot && (tt < 1000 && tt > 0))
            ttPlot->data->add_point(tv_to_curr_time(&strehl_sim.frame_time), tt);
         if(ho1Plot && (ho1 < 1000 && ho1 > 0))
            ho1Plot->data->add_point(tv_to_curr_time(&strehl_sim.frame_time), ho1);
         if(ho2Plot && (ho2 < 1000 && ho2 > 0))
            ho2Plot->data->add_point(tv_to_curr_time(&strehl_sim.frame_time), ho2);
         
         curr_strehl++;
         if(curr_strehl > strehl_sis.get_n_images() - 1)
         {
            curr_strehl = 0;
         }
         i++;
      }
   }


}


} //namespace VisAO
