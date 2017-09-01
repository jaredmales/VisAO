
#ifndef __basic_ui_h__
#define __basic_ui_h__

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QTimer>
#include <QThread>

#include "VisAOApp_standalone.h"
#include "libvisao.h"

namespace VisAO
{

class basic_ui;

///Thread class to start the retreive_state method.
/** Is a separate thread so fifo timeouts, from say the controller being down
  * don't degrade GUI performance.
  */
class MyThread : public QThread
{
public:
   void run();
   basic_ui * bu;
};


///The basic user interface for ViSAO control panels.
/** Manages periodic status updates from the controller.
  */
class basic_ui : public QWidget,  public VisAOApp_standalone
{
   Q_OBJECT
   
public:
   basic_ui(QWidget * Parent = 0, Qt::WindowFlags f = 0);
   basic_ui(int argc, char **argv, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   basic_ui( std::string name, const std::string& conffile, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   void Create();

   MyThread mth;
   
protected:
   QLabel * __LabelCmode;
   QLabel * __StatusCmode;
   QLabel * __StatusConnected;
   QPushButton * __ButtonTakeLocal;
   QCheckBox * __checkBoxOverride;
   
   QTimer statustimer; ///< When this times out check status.
   int statustimeout; ///<The timeout for checking for status.
   
   int state_connected; ///<Whether or not the GUI is connected to the controller.
   
   int state_cmode; ///<The control mode of the controller

   ///This function should be reimplemented in derived GUIs.  DO NOT UPDATE GUI HERE!
   /** Since this runs in a separate thread, it is not safe to update the GUI from this
     * method.  Instead, it should only update variables based on the controller response.
     * It also should not lock \ref commutex, as this is done prior to being called.
     */ 
   virtual void retrieve_state(){return;}

   ///Called by \ref time_to_update() when the \ref state_retrieved() signal is received.
   /** Updates the basic_ui, and calls \ref update_status().
     */
   void basic_update_status();

   ///This function should be reimplemented in derived GUIs.  
   virtual void update_status(){return;}

   pthread_mutex_t upmutex; ///<mutex for waiting on the \ref upcond condition
   pthread_mutex_t commutex; ///<mutex to protect fifo communications
   pthread_cond_t  upcond; ///<condition to tell the state retrieve thread to go.

   QString paramStyle;
   QString upStyle;
   QString warnStyle;
   QString softwarnStyle;
   QString downStyle;
   QString normStyle;
 
   
public:
   void update_thread();
   
protected slots:

   ///Every time the timer timesout, \ref upcond is broadcast
   virtual void statustimerout();

   ///Slot to receive the state_retrieved() signal.
   void time_to_update();

signals:

   ///Signal to announce that the GUI should be updated.
   void state_retrieved();
   
};




}//namespace VisAO

#endif //__basic_ui_h__
