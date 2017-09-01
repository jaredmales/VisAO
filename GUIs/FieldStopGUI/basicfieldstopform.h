

#include "ui_BasicFieldStopForm.h"

#include "VisAOApp_base.h"
#include "libvisao.h"

#include <QWidget>
#include <sstream>
#include <QMessageBox>
#include <QFileDialog>

#include "../basic_ui.h"

#include "AOStates.h"

namespace VisAO
{
   
class BasicFieldStopForm : public VisAO::basic_ui
{
   Q_OBJECT
   
public:
   BasicFieldStopForm(QWidget * Parent = 0, Qt::WindowFlags f = 0);
   BasicFieldStopForm(int argc, char **argv, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   BasicFieldStopForm(std::string name, const std::string& conffile, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   

   void Create();

protected:
   int state_connectedFS; ///<Whether or not the GUI is connected to the controller.
   int state_cmodeFS; ///<The control mode of the controller
   
   int hwpstate;
   int hwppower;
   int hwpmoving;
   int hwphoming;
   double hwpposition;
   
   int fsstate;
   int fspower;
   int fsmoving;
   int fshoming;
   double fsposition;
   
   void attach_basic_ui();
   
   double wait_to;

   void retrieve_state();
   void retrieve_stateFS();
   
   void update_status();

   
   void update_statusFS();
   

   
   
   
   
public:
  
   
protected slots:
   
   
   void on_HWPstopButton_clicked();
   
   void on_HWPminus45_clicked();
   void on_HWPminus225_clicked();
   void on_HWPplus225_clicked();
   void on_HWPplus45_clicked();
   void on_HWPcontinuous_clicked();
   void on_HWPgoButton_clicked();
   void on_HWPhome_clicked();
   
   void on_ButtonTakeLocal_clicked();
   
   
   void on_FSstopButton_clicked();
   void on_FSFieldStop_clicked();
   void on_FSHWP_clicked();
   void on_FSStow_clicked();
   void on_FSgoButton_clicked();
   void on_FShome_clicked();
   
   void on_ButtonTakeLocal_FS_clicked();
private:
   
   Ui::BasicFieldStopForm ui;
   
};

} //namespace VisAO
