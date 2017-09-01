#include "basicccd47form.h"

namespace VisAO
{

BasicCCD47CtrlForm::BasicCCD47CtrlForm(QWidget * Parent, Qt::WindowFlags f) : basic_ui(Parent, f)
{
 
}

BasicCCD47CtrlForm::BasicCCD47CtrlForm(int argc, char **argv, QWidget * Parent, Qt::WindowFlags f) : basic_ui(argc, argv, Parent, f)
{
   Create();
}

BasicCCD47CtrlForm::BasicCCD47CtrlForm(std::string name, const std::string& conffile, QWidget * Parent, Qt::WindowFlags f) : basic_ui(name, conffile, Parent, f)
{
   Create();
}

void BasicCCD47CtrlForm::Create()
{
   shutter = 0;
   
   ui.setupUi(this);
   attach_basic_ui();

   ui.toolBox->setCurrentIndex(1);

   ui.statusSpeed->setStyleSheet(paramStyle);
   ui.statusWindow->setStyleSheet(paramStyle);
   ui.statusBinning->setStyleSheet(paramStyle);
   ui.statusRepetitions->setStyleSheet(paramStyle);
   ui.statusIntegration->setStyleSheet(paramStyle);
   ui.statusFrameRate->setStyleSheet(paramStyle);
   ui.statusGain->setStyleSheet(paramStyle);
   ui.statusBlacks->setStyleSheet(paramStyle);
   ui.statusJoeTemp->setStyleSheet(paramStyle);
   ui.statusHeadTemp1->setStyleSheet(paramStyle);
   ui.statusHeadTemp2->setStyleSheet(paramStyle);
   
   ui.configFPS->setStyleSheet(paramStyle);
   ui.configMinExp->setStyleSheet(paramStyle);
   ui.configMaxExp->setStyleSheet(paramStyle);
   
   ui.imTypeCombo->setLayoutDirection(Qt::LeftToRight);
   ui.comboProgram->setLayoutDirection(Qt::LeftToRight);
   ui.comboGain->setLayoutDirection(Qt::LeftToRight);
   ui.blacksChan1->setLayoutDirection(Qt::LeftToRight);
   ui.blacksChan0->setLayoutDirection(Qt::LeftToRight);
               
   setup_fifo_list(1);

   std::string visao_root = getenv("VISAO_ROOT");
   std::string CCD47Ctrl_fifopath = (std::string)(ConfigDictionary())["CCD47Ctrl_fifopath"];

   std::string fpathin = visao_root + "/" + CCD47Ctrl_fifopath + "/ccd47ctrl_com_local_in";
   std::string fpathout = visao_root + "/" + CCD47Ctrl_fifopath + "/ccd47ctrl_com_local_out";

   std::string ccd47confpath = (std::string)(ConfigDictionary())["ccd_conf"];

   
   ccd47confpath = Utils::getConffile(ccd47confpath);

   

   try
   {
      ccd_conf = new Config_File(ccd47confpath);
   }
   catch(Config_File_Exception &e)
   {
      logss.str("");
      logss << "Error accessing ccd configuration files.";
      log_msg(Logger::LOG_LEV_FATAL, logss.str());
      std::cerr << logss.str() << " (logged) \n";
      throw;
   }

   LoadJoeDiskFiles();

   try
   {
      load_time_short = (ConfigDictionary())["load_time_short"];
   }
   catch(Config_File_Exception &e)
   {
      load_time_short = 0.5;
   }

   try
   {
      load_time_long = (ConfigDictionary())["load_time_long"];
   }
   catch(Config_File_Exception &e)
   {
      load_time_long = 35.;
   }


   signal(SIGIO, SIG_IGN);
   set_fifo_list_channel(&fl, 0, 100,fpathout.c_str(), fpathin.c_str(), 0, 0);

   if(connect_fifo_list() < 0)
   {
      QMessageBox msgBox;
      msgBox.setText("Another CCD47 GUI is probably already running.");
      msgBox.setInformativeText("Cannot lock the input FIFO on one or more channels.");
      msgBox.exec();
      exit(-1);
   }
   
   

   ui.comboGain->insertItem(0, "");
   ui.comboGain->insertItem(1, "High");
   ui.comboGain->insertItem(2, "Med. High");
   ui.comboGain->insertItem(3, "Med. Low");
   ui.comboGain->insertItem(4, "Low");

   
   ui.comboProgram->insertItem(0, "");
   ui.comboProgram->insertItem(1, "Set  Speed       Window        Bin       FPS");
   //ui.comboProgram->insertSeparator(0);
   int k=2;
   for(unsigned i=0; i< ondisk.size(); i++)
   {
      for(unsigned j=0; j < ondisk[i].programs.size(); j++)
      {
         if(ondisk[i].programs[j].binx > 0)
         {
            comboindex_to_progset.push_back(i);
            comboindex_to_program.push_back(j);
            ui.comboProgram->insertItem(k, ondisk[i].programs[j].name.c_str());
            k++;
         }
      }
      //ui.comboProgram->insertSeparator(k-1);
   }
   //progdex = 0;
   
   //ui.spinReps->setValue(0);
   ui.progressLoad->setVisible(false);
   ui.buttonLoad->setVisible(true);
   loading = 0;
   
   
   ui.imTypeCombo->insertItem(0, "Science");
   ui.imTypeCombo->insertItem(1, "Acquisition");
   ui.imTypeCombo->insertItem(2, "Dark");
   ui.imTypeCombo->insertItem(3, "Sky");
   ui.imTypeCombo->insertItem(4, "Flat");
   
   load_time = 1.;
         
   acqmode=0;
   
   statustimerout();
   
   
   
   return;
}

//@Function: LoadJoeDiskFiles
//
// Load the configuration files specifying the different
// program sets that can be uploaded to LittleJoe
//
//@

int BasicCCD47CtrlForm::LoadJoeDiskFiles()
{
   int num, i;
   std::string prefix;

   num = (*ccd_conf)["num_programsets"];

   ondisk.resize(num);

   for (i=0; i<num; i++)
   {
      char param[32];
      sprintf( param, "programset%d", i);
      Config_File *subtree = (*ccd_conf).extract(param);
      ondisk[i] = ReadProgramSet( *subtree);
      delete subtree;
   }

   //if (debug) for (i=0; i<num; i++) DumpProgramset(ondisk[i]);

   _logger->log( Logger::LOG_LEV_INFO, "%d programsets loaded.", ondisk.size());
   return 0;
}


// Reads a configuration file with the parameters of an entire LittleJoe program set

VisAO::littlejoe_programset BasicCCD47CtrlForm::ReadProgramSet( Config_File &cfg)
{
   int size;
   unsigned int i;

   VisAO::littlejoe_programset programset;

   programset.name = (std::string) cfg["name"];
   programset.control_filename = (std::string) cfg["control_filename"];
   programset.pattern_filename = (std::string) cfg["pattern_filename"];
   programset.load_time = cfg["load_time"];

   size = cfg["num_programs"];

   if ((size<=0) || (size > 1000))
   {
      _logger->log( Logger::LOG_LEV_WARNING, "Skipping programset %s because it has %d programs", programset.name.c_str(), size);
      return programset;
   }

   programset.programs.resize( size);

   for (i=0; i< programset.programs.size(); i++)
   {
      char par_name[32];
      sprintf( par_name, "program%d", i);

      _logger->log( Logger::LOG_LEV_DEBUG, "Reading program %d of %d", i, programset.programs.size());
      Config_File *subtree = cfg.extract(par_name);
      programset.programs[i] = ReadProgram( *subtree);
      delete subtree;
   }

   return programset;
}

// Read a configuration file with the parameters of a single LittleJoe program

VisAO::littlejoe_program BasicCCD47CtrlForm::ReadProgram( Config_File &cfg)
{
   VisAO::littlejoe_program program;

   program.name = (std::string) cfg["name"];
   program.readout_speed = cfg["readout_speed"];
   program.binx = cfg["binx"];
   program.biny = cfg["biny"];
   program.windowx = cfg["windowx"];
   program.windowy = cfg["windowy"];
   program.delay_base = cfg["delay_base"];
   program.delay_inc = cfg["delay_inc"];

   // Read default black levels if they exist, otherwise set to -1
   for (int i=0; i<4; i++) 
   {
      try 
      {
         char str[10];
         sprintf( str, "black%d", i+1);
         program.black_levels[i] = cfg[str];
      } 
      catch (Config_File_Exception &e) 
      {
         program.black_levels[i] = -1;
      }
   }

   program.EDT_cfg_fname = (std::string) cfg["EDT_cfg_fname"];
   
   _logger->log( Logger::LOG_LEV_DEBUG, "Found: speed %d, binx %d, biny %d, windowx %d, windowy %d, base %f, inc %f, EDT config %s", program.readout_speed, program.binx, program.biny, program.windowx, program.windowy, program.delay_base, program.delay_inc, program.EDT_cfg_fname.c_str());

   return program;
}

void BasicCCD47CtrlForm::attach_basic_ui()
{
   __LabelCmode = ui.LabelCmode;
   __StatusCmode = ui.StatusCmode;
   __StatusConnected = ui.StatusConnected;
   __ButtonTakeLocal = ui.ButtonTakeLocal;
   __checkBoxOverride = ui.checkBoxOverride;

   ui.cModeFrame->lower();
}


void BasicCCD47CtrlForm::retrieve_state()
{
   std::string resp;
   //static double t0 = get_curr_time();
   //std::cout.precision(8);
   
   if(!loading)
   {
      write_fifo_channel(0, "state?\n", 8, &resp);
      
      if(resp == "" || resp.length() < 3)
      {
         setDisconnected();
         return;
      }
      
      state_connected = 1;
      
      state_cmode = resp[0];
      
      status = atoi(&resp[2]);
      
      write_fifo_channel(0, "set?\n", 6, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setDisconnected();
         return;
      }
      program_set = atoi(resp.c_str());
      
      //std::cerr << get_curr_time() - t0 << " Sending " << "prog?\n";
      write_fifo_channel(0, "prog?\n", 7, &resp);
      //std::cerr << get_curr_time() - t0 << " Done\n";
      //std::cout << resp << "\n";
      if(resp == "" || resp.length() < 1)
      {
         setDisconnected();
         return;
      }
      program = atoi(resp.c_str());
      
      write_fifo_channel(0, "speed?\n", 8, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setDisconnected();
         return;
      }
      speed = atoi(resp.c_str());
      
      //   std::cerr << get_curr_time() - t0 << " Sending " << "windowx?\n";
      write_fifo_channel(0, "windowx?\n", 10, &resp);
      //std::cerr << get_curr_time() - t0 << " Done\n";
      //std::cout << resp << "\n";
      if(resp == "" || resp.length() < 1)
      {
         setDisconnected();
         return;
      }
      windowx = atoi(resp.c_str());
      //   std::cerr << get_curr_time() - t0 << " Sending " << "windowy?\n";
      write_fifo_channel(0, "windowy?\n", 10, &resp);
      //std::cerr << get_curr_time() - t0 << " Done\n";
      //std::cout << resp << "\n";
      if(resp == "" || resp.length() < 1)
      {
         setDisconnected();
         return;
      }
      windowy = atoi(resp.c_str());
      
      write_fifo_channel(0, "xbin?\n", 7, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setDisconnected();
         return;
      }
      xbin = atoi(resp.c_str());
      
      write_fifo_channel(0, "ybin?\n", 7, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setDisconnected();
         return;
      }
      ybin = atoi(resp.c_str());
      
      
      write_fifo_channel(0, "rep?\n", 6, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setDisconnected();
         return;
      }
      repetitions = atoi(resp.c_str());
      
      write_fifo_channel(0, "framerate?\n", 12, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setDisconnected();
         return;
      }
      framerate = strtod(resp.c_str(), 0);
      
      write_fifo_channel(0, "gain?\n", 7, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setDisconnected();
         return;
      }
      gain = atoi(resp.c_str());
      
      
      write_fifo_channel(0, "temps?\n", 8, &resp);
      if(resp == "" || resp.length() < 20)
      {
         setDisconnected();
         return;
      }
      //std::cout << "resp:" << resp << "|\n";
      temp1 = strtod(resp.substr(0,6).c_str(),0);
      //std::cout << "1\n";
      temp2 = strtod(resp.substr(7,6).c_str(),0);
      //std::cout << "2\n";
      temp3 = strtod(resp.substr(14,6).c_str(),0);
      
      
      
      write_fifo_channel(0, "blacks?\n", 8, &resp);
      if(resp == "" || resp.length() < 9)
      {
         setDisconnected();
         return;
      }
      //std::cout << "resp:" << resp << "|\n";
      black0 = atoi(resp.substr(0,4).c_str());
      //std::cout << "1\n";
      black1 = atoi(resp.substr(5,4).c_str());
      
      
      //std::cout << "3\n";
      
      write_fifo_channel(0, "save?\n", 7, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setDisconnected();
         return;
      }
      if(resp[0] == '1') saving = 1;
      else saving = 0;

      write_fifo_channel(0, "remaining?", 10, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setDisconnected();
         return;
      }
      remaining = atoi(resp.c_str());
      
      
      write_fifo_channel(0, "skip?\n", 7, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setDisconnected();
         return;
      }
      skipping = atoi(resp.c_str());
      
      write_fifo_channel(0, "imtype?\n", 9, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setDisconnected();
         return;
      }
      imtype = atoi(resp.c_str());
      
   }
   return;
   
}

void BasicCCD47CtrlForm::setDisconnected()
{
   state_connected = 0;
   state_cmode = -1;

   status = -1;
   program_set = -1;
   program = -1;
   
   speed = -1;
   windowx = -1;
   windowy = -1;
   xbin = -1;
   ybin = -1;
      
   repetitions = -1;
   
   framerate = -1;
   
   gain = -1;
}

void BasicCCD47CtrlForm::update_status()
{
   char tmpstr[100];
   
   if(state_connected == 0 || state_cmode != 'L')
   {
      ui.buttonStart->setEnabled(false);
      
      ui.buttonLoad->setEnabled(false);
      
      ui.comboProgram->setEnabled(false);
      ui.comboProgram->setEnabled(false);
      ui.comboGain->setEnabled(false);
      ui.inputExpTime->setEnabled(false);
      ui.forceCheckBox->setEnabled(false);
      ui.swOnlyCheckBox->setEnabled(false);
      ui.blacksSetButton->setEnabled(false);
      ui.saveDirButton->setEnabled(false);
      ui.imTypeCombo->setEnabled(false);
      ui.inputSaveFrames->setEnabled(false);
      ui.buttonSave->setEnabled(false);
      ui.buttonAcq->setEnabled(false);
      ui.buttonDarks->setEnabled(false);
      ui.checkBoxSaveContinuous->setEnabled(false);
      ui.inputSkipFrames->setEnabled(false);
      
      
   }
   
   if(state_connected == 0 || status == -1)
   {
      if(status == -1)
      {
         ui.statusStatus->setText("unknown");
      }
      ui.statusProgSet->setText("Current Program (unk)");
      ui.statusSpeed->setText("unk");     
      ui.statusWindow->setText("unk");
      ui.statusBinning->setText("unk");
      ui.statusRepetitions->setText("unk");
      ui.statusFrameRate->setText("unk");
      ui.statusIntegration->setText("");
      ui.statusGain->setText("unk");
      ui.statusBlacks->setText("? / ?");
      ui.statusJoeTemp->setText("?");
      ui.statusHeadTemp1->setText("?");
      ui.statusHeadTemp2->setText("?");
      
      
      return;
   }
   
//    if(status == -1)
//    {
//       ui.statusStatus->setText("unknown");
//       ui.buttonStart->setEnabled(false);
//       
//       ui.buttonLoad->setEnabled(false);
//       ui.comboProgram->setEnabled(false);
//       ui.comboProgram->setEnabled(false);
//       ui.inputExpTime->setEnabled(false);
//       ui.forceCheckBox->setEnabled(false);
//       ui.swOnlyCheckBox->setEnabled(false);
//       ui.blacksSetButton->setEnabled(false);
//       ui.saveDirButton->setEnabled(false);
//       ui.imTypeCombo->setEnabled(false);
//       ui.inputSaveFrames->setEnabled(false);
//       ui.buttonSave->setEnabled(false);
//       ui.buttonAcq->setEnabled(false);
//       ui.buttonDarks->setEnabled(false);
//       ui.checkBoxSaveContinuous->setEnabled(false);
//       ui.inputSkipFrames->setEnabled(false);
//       
//    }
   
   
   if(state_cmode == 'L')
   {
      ui.buttonStart->setEnabled(true);
      
      ui.buttonLoad->setEnabled(true);
      ui.comboProgram->setEnabled(true);
      ui.comboProgram->setEnabled(true);
      ui.comboGain->setEnabled(true);
      ui.inputExpTime->setEnabled(true);
      ui.forceCheckBox->setEnabled(true);
      ui.swOnlyCheckBox->setEnabled(true);
      ui.blacksSetButton->setEnabled(true);
     
     
      
      ui.buttonSave->setEnabled(true);
     
      ui.buttonDarks->setEnabled(true);
      
      
   }
      
      
   if(loading)
   {
       ui.statusStatus->setText("Loading");
   }
   else
   {
      if(status == 1)
      {
         ui.statusStatus->setText("stopped");
         ui.buttonStart->setText("Start");
      }
      if(status == 2)
      {
         ui.statusStatus->setText("running");
         ui.buttonStart->setText("Stop");
      }
   }
   
   if(saving == 0)
   {
      ui.buttonSave->setText("Start Saving");
      
      ui.statusSaving->setText("not saving");
      
      if( state_cmode == 'L')
      {
         ui.buttonAcq->setEnabled(true);
      ui.saveDirButton->setEnabled(true);
      ui.saveDirInput->setEnabled(true);
      ui.labelImType->setEnabled(true);
      ui.imTypeCombo->setEnabled(true);
      
      if(ui.checkBoxSaveContinuous->checkState() == Qt::Checked)
      {
         ui.labelSave->setEnabled(false);
         ui.inputSaveFrames->setEnabled(false);
         ui.labelSaveFrames->setEnabled(false);
      }
      else
      {  
         ui.labelSave->setEnabled(true);
         ui.inputSaveFrames->setEnabled(true);
         ui.labelSaveFrames->setEnabled(true);
      }   
      
      ui.checkBoxSaveContinuous->setEnabled(true);
      ui.labelSkip->setEnabled(true);
      ui.inputSkipFrames->setEnabled(true);
      ui.labelSkipFrames->setEnabled(true);
      
      ui.buttonAcq->setEnabled(true);
      }
      
   }
   
   
   if(saving == 1)
   {
      std::string imtstr;
      switch(imtype)
      {
         case 0: imtstr = "SCI";
         break;
         case 1: imtstr = "ACQ";
         break;
         case 2: imtstr = "DARK";
         break;
         case 3: imtstr = "SKY";
         break;
         case 4: imtstr = "FLAT";
         break;
      }
      ui.buttonSave->setText("Stop Saving");
      
      
      if(n_saving == -1) snprintf(tmpstr, 100, "saving cont (%s), skipping %i",imtstr.c_str(), skipping);
      else if(skipping > 0) snprintf(tmpstr, 100, "saving %i (%s), skipping %i, %i remaining", n_saving, imtstr.c_str(), skipping, remaining);
      else snprintf(tmpstr, 100, "saving %i (%s) / %i remaining", n_saving, imtstr.c_str(), remaining);
      ui.statusSaving->setText(tmpstr);
      
      if(state_cmode == 'L')
      {
         ui.buttonAcq->setEnabled(false);
         ui.saveDirButton->setEnabled(false);
         ui.saveDirInput->setEnabled(false);
         ui.labelImType->setEnabled(false);
         ui.imTypeCombo->setEnabled(false);
         ui.labelSave->setEnabled(false);
         ui.inputSaveFrames->setEnabled(false);
         ui.labelSaveFrames->setEnabled(false);
         ui.checkBoxSaveContinuous->setEnabled(false);
         ui.labelSkip->setEnabled(false);
         ui.inputSkipFrames->setEnabled(false);
         ui.labelSkipFrames->setEnabled(false);
      
         ui.buttonAcq->setEnabled(false);
      }
   }
   
   if(program_set == -1)
   {
      ui.statusProgSet->setText("Current Program (unk)");
   }
   else
   {
      snprintf(tmpstr, 100, "Current Program (Set %i)", program_set);
      ui.statusProgSet->setText(tmpstr);
   }
   
   if(speed == -1)
   {
      ui.statusSpeed->setText("unk");
   }
   else
   {
      snprintf(tmpstr, 100, "%i kHz", speed);
      ui.statusSpeed->setText(tmpstr);
   }
   
   if(windowx == -1 || windowy == -1)
   {
      ui.statusWindow->setText("unk");
   }
   else
   {
      snprintf(tmpstr, 100, "%i X %i", windowx, windowy);
      ui.statusWindow->setText(tmpstr);
   }
   
   if(xbin == -1 || ybin == -1)
   {
      ui.statusBinning->setText("unk");
   }
   else
   {
      snprintf(tmpstr, 100, "%i X %i", xbin, ybin);
      ui.statusBinning->setText(tmpstr);
   }
   
   if(repetitions == -1)
   {
      ui.statusRepetitions->setText("unk");
   }
   else
   {
      snprintf(tmpstr, 100, "%i", repetitions);
      ui.statusRepetitions->setText(tmpstr);
   }
   
   
   if(framerate == -1)
   {
      ui.statusFrameRate->setText("unk");
   }
   else
   {
      snprintf(tmpstr, 100, "%0.3f fps", framerate);
      ui.statusFrameRate->setText(tmpstr);
      
      snprintf(tmpstr, 100, "%0.3f sec", 1./framerate);
      ui.statusIntegration->setText(tmpstr);
   }
   
   switch(gain)
   {
      case 0:
         ui.statusGain->setText("High");
         break;
      case 1:
         ui.statusGain->setText("Med. High");
         break;
      case 2:
         ui.statusGain->setText("Med. Low");
         break;
      case 3:
         ui.statusGain->setText("Low");
         break;
      default:
         ui.statusGain->setText("unk");
   }

   ui.imTypeCombo->setCurrentIndex(imtype);
   
   static int last_black0=0, last_black1=0;

   snprintf(tmpstr, 100, "%4i / %4i", black0, black1);
   ui.statusBlacks->setText(tmpstr);

   //Update the spinboxes if the black level changes.
   if(black0 != last_black0) ui.blacksChan0->setValue(black0);
   if(black1 != last_black1) ui.blacksChan1->setValue(black1);

   last_black0 = black0;
   last_black1 = black1;

   
   snprintf(tmpstr, 100, " %.1f C", temp1);
   ui.statusJoeTemp->setText(tmpstr);
   
   snprintf(tmpstr, 100, "%.1f C", temp2);
   ui.statusHeadTemp1->setText(tmpstr);
   
   snprintf(tmpstr, 100, "%.1f C", temp3);
   ui.statusHeadTemp2->setText(tmpstr);
   
   update_loadbar();
}


void BasicCCD47CtrlForm::update_loadbar()
{
   //std::cout << loading << "\n";
   if(loading == 0) return;
   
   if(load_percent > 1. || loading == 1)
   {
      loading = 0;
      load_percent = 0;
      ui.progressLoad->setValue(0);
      ui.progressLoad->setVisible(false);
      ui.buttonLoad->setVisible(true);
      return;
   }

   int k = ui.comboProgram->currentIndex() - 2;

   ui.progressLoad->setVisible(true);
   ui.buttonLoad->setVisible(false);
   
   double t = get_curr_time();
   
   load_percent = t - load_start;
   
   if(loading == 1) load_percent /= load_time_short;
   if(loading == 2) load_percent /= load_time;
   
   ui.progressLoad->setValue((int)(load_percent*100.));
   
}

void BasicCCD47CtrlForm::on_buttonStart_clicked()
{
   std::string resp;

   pthread_mutex_lock(&commutex);
   
   if(status == 1)
   write_fifo_channel(0, "start\n", 7, &resp);

   if(status == 2)
   write_fifo_channel(0, "stop\n", 6, &resp);

   pthread_mutex_unlock(&commutex);
}
   
void BasicCCD47CtrlForm::on_buttonSave_clicked()
{
   char tmpstr[512];
   std::string resp;

   pthread_mutex_lock(&commutex);
   
   if(saving == 0)
   {
      QString subdir = ui.saveDirInput->text();
      snprintf(tmpstr, 512, "subdir %s\n", subdir.toStdString().c_str() );
      write_fifo_channel(0, tmpstr, strlen(tmpstr)+1, &resp);
      
      int nsk = ui.inputSkipFrames->text().toInt();
      if(nsk < 0) nsk = 0;

      snprintf(tmpstr, 50, "skip %i\n", nsk );
      write_fifo_channel(0, tmpstr, strlen(tmpstr)+1, &resp);

      if(ui.checkBoxSaveContinuous->isChecked())
      {
         n_saving = -1;
         remaining = 0;
         write_fifo_channel(0, "save -1\n", 9, &resp);
      }
      else
      {
         n_saving = ui.inputSaveFrames->text().toInt();
         remaining = n_saving;
         
         if(imtype == 2)
         {
            snprintf(tmpstr, 50, "dark %i\n", ui.inputSaveFrames->text().toInt());
            write_fifo_channel(0, tmpstr, strlen(tmpstr)+1, &resp);
         }
         
         snprintf(tmpstr, 50, "save %i\n", ui.inputSaveFrames->text().toInt());
         write_fifo_channel(0, tmpstr, strlen(tmpstr)+1, &resp);
         
         
      }
      saving = 1;
      pthread_mutex_unlock(&commutex);
      return;
   }

   if(saving == 1)
   write_fifo_channel(0, "save 0\n", 8, &resp);

   pthread_mutex_unlock(&commutex);
}
		
void BasicCCD47CtrlForm::on_buttonAcq_clicked()
{
   char tmpstr[128];
   //load button should be disabled in acq mode and if saving
   std::string resp;
   
   if(acqmode == false)
   {
      acq_imtype = imtype;
      acq_set = program_set;
      acq_program = program;
      acq_gain = gain;
      acq_reps = repetitions;
      
      snprintf(tmpstr, 128, "set %i %i %i %i\n", program_set, program, gain, 0);
      
      on_imTypeCombo_currentIndexChanged(1);
   
      ui.buttonAcq->setText("Exit Acquisition");
      acqmode = true;
   }
   else
   {
      snprintf(tmpstr, 128, "set %i %i %i %i\n", acq_set, acq_program, acq_gain, acq_reps);
      
      on_imTypeCombo_currentIndexChanged(acq_imtype);
      
      ui.buttonAcq->setText("Acquisition");
      acqmode = false;
   }
   
   load_start = get_curr_time();

   pthread_mutex_lock(&commutex);
   
   ///Don't wait for response, since the CCD47Ctrl doesn't respond until load is done.
   write_fifo_channel(0, tmpstr, strlen(tmpstr) + 1, &resp);

   pthread_mutex_unlock(&commutex);
   
   load_percent = 0;
   
}
      
      
void BasicCCD47CtrlForm::on_buttonLoad_clicked()
{
   std::string resp;
   char tmpstr[100];
   int k = ui.comboProgram->currentIndex()-2;
   int g = ui.comboGain->currentIndex()-1;
   
   if(k < 0) 
   {
      k = 0;
      while(!(comboindex_to_progset[k] == program_set && comboindex_to_program[k] == program))
      {
         k++;
         if (k >= comboindex_to_program.size())
         {
            k = 0;
            break;
         }
      }

      
   }
   
   if(g < 0) 
   {
      g = gain;
   }
   
   double et = strtod(ui.inputExpTime->text().toStdString().c_str(), 0);
   if(et < minITime) et = minITime;
   if(et > maxITime) et = maxITime;
    
   int r = ComputeRepsExpTime(ondisk[comboindex_to_progset[k]].programs[comboindex_to_program[k]].delay_base, ondisk[comboindex_to_progset[k]].programs[comboindex_to_program[k]].delay_inc, et);
//   int r = ui.spinReps->value();

   int ps, pr;

   //std::cout << "1\n";
   ps = comboindex_to_progset[k];
   //std::cout << "2\n";
   pr = comboindex_to_program[k];
   //std::cout << "3\n";

   if(ui.swOnlyCheckBox->checkState() == Qt::Checked && ui.forceCheckBox->checkState() == Qt::Unchecked)
   {
      snprintf(tmpstr, 100, "swset %i %i %i %i\n", comboindex_to_progset[k], comboindex_to_program[k], g, r);
   }
   else if(ui.swOnlyCheckBox->checkState() == Qt::Unchecked && ui.forceCheckBox->checkState() == Qt::Checked)
   {
      snprintf(tmpstr, 100, "fset %i %i %i %i\n", comboindex_to_progset[k], comboindex_to_program[k], g, r);
      loading = 2;
   }
   else
   {
      snprintf(tmpstr, 100, "set %i %i %i %i\n", comboindex_to_progset[k], comboindex_to_program[k], g, r);
      if(comboindex_to_progset[k] != program_set) loading = 2;
      else loading = 1;
   }
   //std::cout << "4\n";
   load_time = ondisk[comboindex_to_progset[k]].load_time;
   load_start = get_curr_time();

   pthread_mutex_lock(&commutex);
   
   ///Don't wait for response, since the CCD47Ctrl doesn't respond until load is done.
   write_fifo_channel(0, tmpstr, strlen(tmpstr) + 1, &resp);

   pthread_mutex_unlock(&commutex);
   
   ui.swOnlyCheckBox->setCheckState(Qt::Unchecked);
   ui.forceCheckBox->setCheckState(Qt::Unchecked);
   //std::cout << "5\n";
   load_percent = 0;

   
   ui.comboProgram->setCurrentIndex(0);
   ui.inputExpTime->setText("");
   ui.comboGain->setCurrentIndex(0);
   
   //std::cout << "6\n";
   //std::cout << tmpstr << "\n";
}

void BasicCCD47CtrlForm::on_buttonDarks_clicked()
{
   char tmpstr[50];
   std::string resp;

   pthread_mutex_lock(&commutex);
   
   snprintf(tmpstr, 50, "dark %i\n", ui.inputDarkFrames->text().toInt());
   //std::cout << tmpstr << "\n";
   write_fifo_channel(0, tmpstr, strlen(tmpstr)+1, &resp);
   
   pthread_mutex_unlock(&commutex);

}


void BasicCCD47CtrlForm::on_forceCheckBox_stateChanged(int state)
{
   if(state == Qt::Checked)
   {
      ui.swOnlyCheckBox->setCheckState(Qt::Unchecked);
   }
}
     
void BasicCCD47CtrlForm::on_swOnlyCheckBox_stateChanged(int state)
{
   if(state == Qt::Checked)
   {
      ui.forceCheckBox->setCheckState(Qt::Unchecked);
   }
}

void BasicCCD47CtrlForm::on_checkBoxSaveContinuous_stateChanged(int state)
{
   if(state == Qt::Checked)
   {
      ui.labelSave->setEnabled(false);
      ui.inputSaveFrames->setEnabled(false);
      ui.labelSaveFrames->setEnabled(false);
   }
   else
   {
      ui.labelSave->setEnabled(true);
      ui.inputSaveFrames->setEnabled(true);
      ui.labelSaveFrames->setEnabled(true);
   }
}

void BasicCCD47CtrlForm::on_blacksSetButton_clicked()
{
   int blev0, blev1;
   char tmpstr[100];
   std::string resp;

   blev0 = ui.blacksChan0->value();
   blev1 = ui.blacksChan1->value();

   pthread_mutex_lock(&commutex);
   
   snprintf(tmpstr, 100, "black 0 %i\n", blev0);

   write_fifo_channel(0, tmpstr, strlen(tmpstr)+1, &resp);

   snprintf(tmpstr, 100, "black 1 %i\n", blev1);

   write_fifo_channel(0, tmpstr, strlen(tmpstr)+1, &resp);

   pthread_mutex_unlock(&commutex);
}

void BasicCCD47CtrlForm::on_ButtonTakeLocal_clicked()
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


void BasicCCD47CtrlForm::on_comboProgram_currentIndexChanged(int k)
{
   double fr;
   char tmpstr[25];
   
   
   if(k < 2) return;
   
   k = k - 2;
   
   int pset  = comboindex_to_progset[k];
   int pgram = comboindex_to_program[k];

   
   fr = ComputeFramerate(ondisk[comboindex_to_progset[k]].programs[comboindex_to_program[k]].delay_base,
                        ondisk[comboindex_to_progset[k]].programs[comboindex_to_program[k]].delay_inc, 0);
   minITime = 1./fr;
   
   fr = ComputeFramerate(ondisk[comboindex_to_progset[k]].programs[comboindex_to_program[k]].delay_base,
                         ondisk[comboindex_to_progset[k]].programs[comboindex_to_program[k]].delay_inc, 65535);
   maxITime = 1./fr;
   
   snprintf(tmpstr, 25, "%0.3f sec", minITime);
   ui.configMinExp->setText(tmpstr);
   
   snprintf(tmpstr, 25, "%0.3f sec", maxITime);
   ui.configMaxExp->setText(tmpstr);
   
   if(ui.inputExpTime->text() == "") 
   {
      double it;
      
      if(pset == program_set && pgram == program)
      {
         it = ( (double) ( (int) (10000./framerate + .5 ) )) / 10000.;
      }
      else
      {
         it = ( (double) ( (int) (10000./(1./minITime) + .5 ) )) / 10000.;
      }
      
      snprintf(tmpstr, 25, "%0.3f", it);
      ui.inputExpTime->setText(tmpstr);
      
         
   }
      
   on_inputExpTime_textChanged(ui.inputExpTime->text());
   
   if(ui.comboGain->currentIndex() == 0) ui.comboGain->setCurrentIndex(gain+1);
         
  
   return;
}

void BasicCCD47CtrlForm::on_comboGain_currentIndexChanged(int g)
{
   double fr;
   char tmpstr[25];
   
   if(g < 1) return;
      
   if(ui.comboProgram->currentIndex() <2) 
   {
      int k = 0;
      
      while(!(comboindex_to_progset[k] == program_set && comboindex_to_program[k] == program))
      {
         k++;
         if (k >= comboindex_to_program.size())
         {
            k = 0;
            break;
         }
      }
      ui.comboProgram->setCurrentIndex(k+2);
   }         
  
   return;
}

void BasicCCD47CtrlForm::on_inputExpTime_textChanged(const QString & text)
{
   char tmpstr[25];
   double et = strtod(text.toStdString().c_str(), 0);
   if(et < minITime) et = minITime;
   if(et > maxITime) et = maxITime;
   
   snprintf(tmpstr, 25, "%0.3ffps", 1./et);
   ui.configFPS->setText(tmpstr);
}
      
void BasicCCD47CtrlForm::on_imTypeCombo_currentIndexChanged(int k)
{
   char tmpstr[25];
   std::string resp;
   
   pthread_mutex_lock(&commutex);
   
   snprintf(tmpstr, 25, "imtype %i\n", k);

   write_fifo_channel(0, tmpstr, strlen(tmpstr)+1, &resp);
   
   pthread_mutex_unlock(&commutex);
   
   if(shutter)
   {
      if(k == 2) shutter->shut();
      else shutter->open();
   }
   
   return;
}

void BasicCCD47CtrlForm::on_imviewerButton_clicked()
{
   system("$VISAO_ROOT/bin/imviewer &");
}

void BasicCCD47CtrlForm::on_saveDirButton_clicked()
{
   QString dirName = QFileDialog::getExistingDirectory(this, tr("Save to directory"), "/home/aosup/visao/data/ccd47");
   
   if(!dirName.startsWith("/home/aosup/visao/data/ccd47"))
   {
      QMessageBox msgBox;
      msgBox.setText("Invalid CCD47 Data Directory");
      msgBox.setInformativeText("CCD47 data can only be saved in ~/visao/data/ccd47/ and subdirectories.");
      msgBox.exec();
      return;
   }
   
   dirName.remove("/home/aosup/visao/data/ccd47");
   
   if(dirName[0] == '/') dirName.remove(0,1);
            
   ui.saveDirInput->setText(dirName);
   
}

}//namespace VisAO
