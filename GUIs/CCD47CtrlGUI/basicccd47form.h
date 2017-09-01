

#include "ui_BasicCCD47Form.h"


#include <iostream>
#include <sstream>
#include <cmath>

#include "../basic_ui.h"
#include "CCD47Ctrl.h"
#include <QMessageBox>
#include <QFileDialog>

#ifndef __basicccd47form_h__
#define __basicccd47form_h__

#include "../ShutterGUI/shutterform.h"

namespace VisAO
{

class BasicCCD47CtrlForm : public VisAO::basic_ui
{
   Q_OBJECT

   public:
      BasicCCD47CtrlForm(QWidget * Parent = 0, Qt::WindowFlags f = 0);
      BasicCCD47CtrlForm(int argc, char **argv, QWidget * Parent = 0, Qt::WindowFlags f = 0);
      BasicCCD47CtrlForm(std::string name, const std::string& conffile, QWidget * Parent = 0, Qt::WindowFlags f = 0);

      void Create();
      
      void setDisconnected();
      
      ShutterForm * shutter;
      
   protected:
      void attach_basic_ui();
      
      Config_File *ccd_conf;
      
      // Struct array representing files that can be uploaded to LittleJoe (each one is a memory dump)
      std::vector<VisAO::littlejoe_programset> ondisk;
      int _startProgramSet;
      
      int LoadJoeDiskFiles(void);
      VisAO::littlejoe_programset ReadProgramSet( Config_File &cfg);
      VisAO::littlejoe_program ReadProgram( Config_File &cfg);
      
      ///Vector to map combo box index to program set, allowing us to skip empty programs
      std::vector<int> comboindex_to_progset;
      ///Vector to map combo box index to program, allowing us to skip empty programs
      std::vector<int> comboindex_to_program;
      
      int status;
      int saving;
      int n_saving;
      int remaining;
      int skipping;
      int imtype;
      
      int program_set;
      int program;
      
      
      int speed;
      int windowx;
      int windowy;
      
      int xbin;
      int ybin;
      
      int repetitions;
      
      
      
      double framerate;
      
      int gain;
      
      double temp1, temp2, temp3;
      int black0, black1;
      
      void retrieve_state();
      
      void update_status();
      
      int loading;
      double load_time;
      double load_time_short;
      double load_time_long;
      
      double load_start;
      double load_percent;
      
      void update_loadbar();
      double minITime, maxITime;
      
      bool acqmode;
      int acq_imtype;
      int acq_set;
      int acq_program;
      int acq_gain;
      int acq_reps;
      
      
      
   protected slots:
      
      void on_buttonStart_clicked();
      void on_buttonSave_clicked();
      void on_buttonAcq_clicked();
      
      void on_buttonLoad_clicked();
      void on_buttonDarks_clicked();
      
      void on_forceCheckBox_stateChanged(int state);
      void on_swOnlyCheckBox_stateChanged(int state);
      void on_checkBoxSaveContinuous_stateChanged(int state);
      
      void on_blacksSetButton_clicked();
      
      void on_ButtonTakeLocal_clicked();
      
      //void on_spinReps_valueChanged(int i);
      void on_comboProgram_currentIndexChanged(int i);
      void on_comboGain_currentIndexChanged(int i);
      
      void on_inputExpTime_textChanged(const QString & text);

      void on_imviewerButton_clicked();
      
      void on_imTypeCombo_currentIndexChanged(int i);
      
      void on_saveDirButton_clicked();
      
   private:

      Ui::BasicCCD47Ctrl ui;

};

} //namespace VisAO

#endif //__basicccd47form_h__
