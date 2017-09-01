

#include "ui_ShutterForm.h"


#include <iostream>
#include <sstream>
#include <QFileDialog>
#include <QString>
#include <QMessageBox>
#include <string>

#include "readcolumns.h"

#include "../basic_ui.h"

#ifndef __shutterform_h__
#define __shutterform_h__

namespace VisAO
{
   
class ShutterForm : public VisAO::basic_ui
{
   Q_OBJECT

public:
   ShutterForm(QWidget * Parent = 0, Qt::WindowFlags f = 0);
   ShutterForm( int argc, char **argv, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   ShutterForm(std::string name, const std::string &conffile, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   
   void Create();
   
   void shut();
   void open();

protected:
   
   void attach_basic_ui();
   
   double wait_to;
   
   int curr_state;
   int sw_state;
   int hw_state;
   int pwr_state;
         
   
   int statustimeout_normal;
   
   void retrieve_state();
   void not_connected();
   
   
   void update_status();
   
   
protected slots:
   
   void on_pushButtonOpenShut_clicked();
      
   void on_ButtonTakeLocal_clicked();
    
   
private:
   
   Ui::Shutter ui;
   
};

}//namespace VisAO

#endif

