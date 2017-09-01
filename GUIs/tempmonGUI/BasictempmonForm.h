

#include "ui_BasictempmonForm.h"


#include <iostream>
#include <sstream>
#include <cmath>

#include <QWidget>
#include <QLabel>
#include <QTimer>

#include "VisAOApp_standalone.h"
#include "libvisao.h"
#include "statusboard.h"

namespace VisAO
{

class BasictempmonForm : public QWidget,  public VisAO::VisAOApp_standalone
{
   Q_OBJECT

public:
   BasictempmonForm(QWidget * Parent = 0, Qt::WindowFlags f = 0);
   BasictempmonForm(int argc, char **argv, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   BasictempmonForm(std::string name, const std::string& conffile, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   
   void Create();
   
protected:
   
   QTimer statustimer; ///< When this times out check status.
   int statustimeout; ///<The timeout for checking for status.
   
   ccd47_status_board * ccd47sb;

   double core_temps[SYS_N_CORES];
   double core_avg;
   double core_temp_warn;
   double core_temp_limit;
 
   double GPUTemp;
   double gpu_temp_warn;
   double gpu_temp_limit;

   double HDDTemp_a;
   double HDDTemp_b;
   double hdd_temp_warn;
   double hdd_temp_limit;

   double AirTemp;
   double air_temp_warn;
   double air_temp_limit;


   double ExhTemp;
   double JoeIntTemp;
   double joe_temp_warn;
   double joe_temp_limit;
   
   double Temp471;
   double Temp472;


protected slots:
   
   void update_status();
   
private:
   
   Ui::Basictempmon ui;
   
};

}//namespace VisAO

