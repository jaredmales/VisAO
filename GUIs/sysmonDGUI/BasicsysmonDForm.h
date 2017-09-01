

#include "ui_basicsysmonDForm.h"


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

class BasicsysmonDForm : public QWidget,  public VisAO::VisAOApp_standalone
{
   Q_OBJECT

public:
   BasicsysmonDForm(QWidget * Parent = 0, Qt::WindowFlags f = 0);
   BasicsysmonDForm(int argc, char **argv, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   BasicsysmonDForm(std::string name, const std::string& conffile, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   
   void Create();
   
protected:
   
   QTimer statustimer; ///< When this times out check status.
   int statustimeout; ///<The timeout for checking for status.
   
   double core_temps[SYS_N_CORES];
   double core_max[SYS_N_CORES];
   double core_idle[SYS_N_VCORES];
   
   size_t mem_tot;
   size_t mem_used;
   size_t mem_free;
   size_t mem_shared;
   size_t mem_buff;
   size_t mem_cached;
   
   size_t swap_tot;
   size_t swap_used;
   size_t swap_free;
   
   size_t dfroot_size;
   size_t dfroot_used;
   size_t dfroot_avail;
   
protected slots:
   
   void update_status();
   
private:
   
   Ui::BasicsysmonD ui;
   
};

}//namespace VisAO

