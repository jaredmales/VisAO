

#include "QLayout"
#include "QMessageBox"

#include "ui_BasicFocusStageForm.h"

#include <iostream>
#include <sstream>
#include <cmath>

#include "../basic_ui.h"

#ifndef __basicfocusstageform_h__
#define __basicfocusstageform_h__

namespace VisAO
{
   
class BasicFocusStageForm : public VisAO::basic_ui
{
   Q_OBJECT

public:
   BasicFocusStageForm(QWidget * Parent = 0, Qt::WindowFlags f = 0);
   BasicFocusStageForm(int argc, char **argv, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   BasicFocusStageForm(std::string name, const std::string& conffile, QWidget * Parent = 0, Qt::WindowFlags f = 0);

   void postCreate(const std::string& conffile);
   
public:
   void Create();
   
protected:
   void attach_basic_ui();
   
   double wait_to;
   
   double state_pos;
   int state_power;
   int state_enabled;
   
   int state_is_moving;
   int state_homing;
   int state_neg_limit;
   int state_home_switch;
   int state_pos_limit;
   int state_pos_limit_disabled;
   
   double state_remaining;
   
   double offset;
   
   void retrieve_state();
   
   void update_status();


public slots:
   
   void on_ButtonGO_clicked();
   void on_ButtonPos_clicked();
   void on_ButtonNeg_clicked();
   void on_ButtonAbort_clicked();

   void on_ButtonGOPreset_clicked();

   void on_ButtonHome_clicked();
   void on_ButtonHomeNeg_clicked();
   void on_ButtonHomePos_clicked();
   
   void on_buttonEnable_clicked();
   
   //void on_lineEditPosition_editingFinished();
   
   void on_lineEditOffset_editingFinished();
   
   void on_ButtonTakeLocal_clicked();
   
   
   
private:

   Ui::BasicFocusStage ui;

};

}

#endif //__basicfocusstageform_h__
