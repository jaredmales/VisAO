

#include "ui_CoronGuideForm.h"


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
   
class CoronGuideForm : public VisAO::basic_ui
{
   Q_OBJECT

public:
   CoronGuideForm(QWidget * Parent = 0, Qt::WindowFlags f = 0);
   CoronGuideForm( int argc, char **argv, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   CoronGuideForm(std::string name, const std::string &conffile, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   
   void Create();

protected:
   
   void attach_basic_ui();
   
   double wait_to;
   
   int loop_state;
   double loop_gain;
   double tgtx;
   double tgty;
   double xcen;
   double ycen;
      
   void retrieve_state();
   void not_connected();
   
   
   void update_status();
   
   int bgAlgorithm;
   double kfwhm;
   int ksize;
   
   bool useAvg;
   bool minMaxRej;
   int avgLen;
   
protected slots:
   
   void on_buttonClosePause_clicked();
   void on_buttonOpen_clicked();
   void on_buttonSetGain_clicked();
   
   void on_kfwhmSetButton_clicked();
   
   void on_bgAlgoCombo_activated(int k);
   
   void on_avgSetButton_clicked();
   
   void on_ButtonTakeLocal_clicked();
    
   
private:
   
   Ui::CoronGuide ui;
   
};

}//namespace VisAO
