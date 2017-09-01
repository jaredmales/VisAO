

#include "ui_BasicWollastonLiftForm.h"

#include "VisAOApp_base.h"
#include "libvisao.h"

#include <QWidget>
#include <sstream>
#include <QMessageBox>

#include "../basic_ui.h"

namespace VisAO
{
   
class BasicWollastonLiftForm : public VisAO::basic_ui
{
   Q_OBJECT
   
public:
   BasicWollastonLiftForm(QWidget * Parent = 0, Qt::WindowFlags f = 0);
   BasicWollastonLiftForm(int argc, char **argv, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   BasicWollastonLiftForm(std::string name, const std::string& conffile, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   

   void Create();

protected:
   
   void attach_basic_ui();
   
   double wait_to;
   
   int state_pos;

   int state_prompt;
   
   void retrieve_state();
   
   void update_status();
   void check_prompt();
   bool prompt_msg_open;
   double prompt_dead_time;
   
protected slots:
   
   void on_upButton_clicked();
   void on_downButton_clicked();
   
   void on_ButtonTakeLocal_clicked();
   
   
private:
   
   Ui::BasicWollastonLiftForm ui;
   
};

} //namespace VisAO
