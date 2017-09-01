

#include "ui_ShutterTesterForm.h"


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
   
class ShutterTesterForm : public VisAO::basic_ui
{
   Q_OBJECT

public:
   ShutterTesterForm(QWidget * Parent = 0, Qt::WindowFlags f = 0);
   ShutterTesterForm( int argc, char **argv, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   ShutterTesterForm(std::string name, const std::string &conffile, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   
   void Create();

protected:
   
   void attach_basic_ui();
   
   double wait_to;
   
   double freq;
   double duration;
   double dutycyc;
   int testmode;
   int curr_state;
   
   double t_remaining;
   int testing;
   
   int statustimeout_normal;
   
   void retrieve_state();
   void not_connected();
   
   
   void update_status();
   
   
   double * strehls;
   int n_strehls;
   
   std::string testfile;
   std::string old_testfile;
   double minStrehl;
   double maxStrehl;
   double threshold;
   double deltat;
   
   double dutycycle;
   
   void update_simstats();
protected slots:
   
   void on_pushButtonStart_clicked();
   void on_pushButtonStop_clicked();
   
   void on_lineEditDuration_editingFinished();
   void on_lineEditFreq_editingFinished();
   void on_lineEditDC_editingFinished();
   
   void on_ButtonTakeLocal_clicked();
   
   void on_pushButtonOpenShut_clicked();
   
   void on_simFileSelect_clicked();
   
   void on_thresholdEntry_editingFinished();
   
   void on_comboBoxTestMode_activated(int tmode);
   
private:
   
   Ui::ShutterTester ui;
   
};

}//namespace VisAO
