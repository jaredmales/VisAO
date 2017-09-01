

#include "ui_FrameSelectorForm.h"


#include <iostream>
#include <sstream>
#include <QFileDialog>
#include <QString>
#include <QMessageBox>
#include <string>

#include "readcolumns.h"

#include "../basic_ui.h"

namespace VisAO
{
   
class FrameSelectorForm : public VisAO::basic_ui
{
   Q_OBJECT

public:
   FrameSelectorForm(QWidget * Parent = 0, Qt::WindowFlags f = 0);
   FrameSelectorForm( int argc, char **argv, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   FrameSelectorForm(std::string name, const std::string &conffile, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   
   void Create();

protected:
   
   void attach_basic_ui();
   
   double wait_to;
   
   int curr_state;
   double curr_thresh;
            
   void retrieve_state();
   void not_connected();
   
   void update_status();
   
   
protected slots:
   
   void on_rtfsStart_clicked();
      
   void on_newThreshButton_clicked();
    
   void on_ButtonTakeLocal_clicked();
   
private:
   
   Ui::FrameSelector ui;
   
};

}//namespace VisAO
