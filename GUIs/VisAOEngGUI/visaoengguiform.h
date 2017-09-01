#include "ui_VisAOEngGUI.h"
#include "VisAOApp_standalone.h"
#include "BasicsysmonDForm.h"
#include "BasictempmonForm.h"
#include "statusboard.h"

namespace VisAO
{
   
class VisAOEngGUIForm : public QDialog,  public VisAOApp_standalone
{
   Q_OBJECT

public:
   VisAOEngGUIForm(QWidget * Parent = 0, Qt::WindowFlags f = 0);
   VisAOEngGUIForm(int argc, char **argv, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   VisAOEngGUIForm(std::string name, const std::string& conffile, QWidget * Parent = 0, Qt::WindowFlags f = 0);

protected:
   QTimer statustimer; ///< When this times out check status.
   int statustimeout; ///<The timeout for checking for status.
   
private:
   ///Initialize the sub forms in the tabs.
   void Create();

protected:
   BasicsysmonDForm * sysmonD;
   BasictempmonForm * tempmon;

   ccd47_status_board * ccd47sb;
   basic_status_board * fg47sb;
   basic_status_board * fw47sb;
   basic_status_board * fs47sb;
   filterwheel_status_board * fw2sb;
   filterwheel_status_board * fw3sb;
   wollaston_status_board *wsb;
   gimbal_status_board *gsb;
   basic_status_board *diosb;
   focusstage_status_board *fsb;
   shutter_status_board *ssb;
   system_status_board *syssb;
   framegrabber39_status_board * fg39sb;
   basic_status_board * fw39sb;
   basic_status_board * recsb;
   aosystem_status_board *visaoisb;
   power_status_board *psb;
   frameselector_status_board *fssb;
   coronguide_status_board *cgsb;
   
   QString paramStyle;
   QString upStyle;
   QString warnStyle;
   QString softwarnStyle;
   QString downStyle;
   QString normStyle;
   
   void check_watchdog(basic_status_board *bsb, QLabel * ql, double ct = 0);
   
   double gimb_flashdelay;
   double gimb_flashstart;
   double gimb_lastx;
   double gimb_lasty;
   
   double focus_flashdelay;
   double focus_flashstart;
   
protected slots:
   ///Launches the sys_processes.py GUI
   void on_processesButton_clicked();

   ///Launches the full systemmonitor GUI
   void on_tempmonButton_clicked();

   ///Launches the full systemmonitor GUI
   void on_sysmonDButton_clicked();

   //void on_gotoPreset_clicked();
   //void on_savePreset_clicked();
   
   ///Every time the timer timesout.
   void statustimerout();

   
   
private:

   Ui::VisAOEngGUI ui;
};

}

