

#include "ui_BasicGimbalForm.h"

#include "VisAOApp_base.h"
#include "libvisao.h"

#include <QWidget>
#include <sstream>
#include <QMessageBox>

#include "../basic_ui.h"
#include "AOStates.h"

namespace VisAO
{
   
class BasicGimbalForm : public VisAO::basic_ui
{
   Q_OBJECT
   
public:
   BasicGimbalForm(QWidget * Parent = 0, Qt::WindowFlags f = 0);
   BasicGimbalForm(int argc, char **argv, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   BasicGimbalForm(std::string name, const std::string& conffile, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   
   void Create();

protected:
   
   void attach_basic_ui();
   
   double wait_to;

   void retrieve_state();
   
   void update_status();

   int curState;
   int pwrState;

   double scale;
   
   double xPos;
   bool xMoving;
   double yPos;
   bool yMoving;

   double xLimit;
   double yLimit;
   
   double slew_lag;
   double small_move;
   //double bigMove;
   
   bool upPressed;
   double timeUpPressed;

   bool downPressed;
   double timeDownPressed;

   bool leftPressed;
   double timeLeftPressed;

   bool rightPressed;
   double timeRightPressed;
   
public slots:
   void on_ButtonTakeLocal_clicked();
   
   void on_upButton_pressed();
   void on_upButton_released();

   void on_downButton_pressed();
   void on_downButton_released();

   void on_leftButton_pressed();
   void on_leftButton_released();

   void on_rightButton_pressed();
   void on_rightButton_released();

   void on_stopButton_clicked();
   
   void on_centerButton_clicked();
   void on_darkButton_clicked();

   void on_goXButton_clicked();
   
   void on_goYButton_clicked();

   void on_savePresetButton_clicked();
   
private:
   
   Ui::BasicGimbalForm ui;
   
};

} //namespace VisAO
