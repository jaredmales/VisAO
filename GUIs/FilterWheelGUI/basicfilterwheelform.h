

//#include "QLayout"
#include "QMessageBox"

#include "ui_BasicFilterWheelForm.h"


#include <iostream>
#include <sstream>
#include <cmath>

#include "../basic_ui.h"

#include "AOStates.h"

#ifndef __basicfilterwheel_h__
#define __basicfilterwheel_h__

namespace VisAO
{

class BasicFilterWheelForm : public VisAO::basic_ui
{
   Q_OBJECT

public:
   BasicFilterWheelForm(QWidget * Parent = 0, Qt::WindowFlags f = 0);
   BasicFilterWheelForm(int argc, char **argv, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   BasicFilterWheelForm(std::string name, const std::string& conffile, QWidget * Parent = 0, Qt::WindowFlags f = 0);

   void postCreate(const std::string& conffile);
   
public:
   void Create();
   
protected:
   void attach_basic_ui();

   std::string FilterWheel_name;
   std::string fw_name;
   Config_File * fw_conf;

   /// Custom positions mapping of name to position
   std::vector<std::string> customPosName;
   std::vector<double> customPos;
   
   double wait_to;
   
   int state;

   double state_pos;
   
   int state_is_moving;
   int state_homing;
   std::string state_filter;
   
   double offset;
   
   void retrieve_state();
   
   void update_status();
   
protected slots:
   
   void on_ButtonGO_clicked();

   void on_ButtonGOFilter_clicked();
   
   void on_ButtonAbort_clicked();
   
   void on_ButtonHome_clicked();

   void on_ButtonPos_clicked();
   void on_ButtonNeg_clicked();

   void on_lineEditOffset_editingFinished();
   
   void on_ButtonTakeLocal_clicked();
     
private:

   Ui::BasicFilterWheel ui;

};

} //namespace VisAO

#endif //__basicfilterwheel_h__
