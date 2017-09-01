
#include "basic_ui.h"

namespace VisAO
{

basic_ui::basic_ui(QWidget * Parent, Qt::WindowFlags f) : QWidget(Parent, f)
{
   Create();
}

basic_ui::basic_ui( int argc, char **argv,QWidget * Parent, Qt::WindowFlags f) : QWidget(Parent, f), VisAOApp_standalone(argc, argv)
{
   Create();
}

basic_ui::basic_ui(std::string name, const std::string& conffile, QWidget * Parent, Qt::WindowFlags f) : QWidget(Parent, f), VisAOApp_standalone(name, conffile)
{
   Create();
}

void basic_ui::Create()
{
   statustimeout = 1000;
   connect(&statustimer, SIGNAL(timeout()), this, SLOT(statustimerout()));

   pthread_mutex_init(&commutex, NULL);

   pthread_mutex_init(&upmutex, NULL);

   pthread_cond_init(&upcond, NULL);

   mth.bu = this;
   mth.start();

   connect(this, SIGNAL(state_retrieved()), this, SLOT(time_to_update()), Qt::QueuedConnection);
   
   statustimer.start(statustimeout);

   paramStyle = "background-color : lightgrey; color : black; ";
   upStyle = "background-color : lime; color : black; ";
   warnStyle = "background-color : yellow; color : black; ";
   softwarnStyle = "background-color : yellowgreen; color : black; ";
   downStyle = "background-color : red; color : black; ";
   normStyle = "";
   
   
}

void basic_ui::basic_update_status()
{
   if(state_connected == 0)
   {
      __StatusConnected->setStyleSheet("color: red");
      __StatusConnected->setText("GUI not connected");
   }
   else
   {
      __StatusConnected->setStyleSheet("color: gray");
      __StatusConnected->setText("GUI connected");
   }

   switch(state_cmode)
   {
      case 'N':
         __StatusCmode->setText("NONE");
         break;
      case 'R':
         __StatusCmode->setText("REMOTE");
         break;
      case 'L':
         __StatusCmode->setText("LOCAL");
         break;
      case 'S':
         __StatusCmode->setText("SCRIPT");
         break;
      case 'A':
         __StatusCmode->setText("AUTO");
         break;
      default:
         __StatusCmode->setText("UNK");
         break;
   }
   
   if(state_cmode == 'L')
   {
      __ButtonTakeLocal->setText("Give Up Local Control");
      __checkBoxOverride->setChecked(false);
      __checkBoxOverride->setEnabled(false);
   }
   else
   {
      __ButtonTakeLocal->setText("Take Local Control");
      __checkBoxOverride->setEnabled(true);
   }
   
   update_status();
}

void basic_ui::update_thread()
{
   //First block all signals in this thread
   sigset_t set;
   sigemptyset(&set);
   sigaddset(&set, SIGIO);
   
   pthread_sigmask(SIG_BLOCK, &set, 0);

   //Now wait on the update condition, until the main thread is termed.
   while(!TimeToDie)
   {
      pthread_mutex_lock(&upmutex);
      pthread_cond_wait(&upcond, &upmutex);
      pthread_mutex_unlock(&upmutex);

      if(pthread_mutex_trylock(&commutex) == 0)
      {
         retrieve_state();
         emit state_retrieved();
         pthread_mutex_unlock(&commutex);
      }
   }
}

void basic_ui::statustimerout()
{
   pthread_cond_broadcast(&upcond);
}

void basic_ui::time_to_update()
{
   basic_update_status();
}


void MyThread::run()
{
   bu->update_thread();
}


} //namespace VisAO
