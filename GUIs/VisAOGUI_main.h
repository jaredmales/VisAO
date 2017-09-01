/************************************************************
*    VisAOGUI_main.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for a VisAO GUI application.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file VisAOGUI_main.h
  * \author Jared R. Males
  * \brief The main program for a VisAO GUI application.
  * 
  * Define VISAO_APP_TYPE and VISAO_APP_CONFFILE before including this in your main program.
  * Then just call return VisAOGUI_main(argc, argv) as the only line in main()
  * e.g:
  *    #define VISAO_APP_TYPE VisAO::FocusMotorCtrl
  *    #define VISAO_APP_NAME "FocusMotor"
  *    #define VISAO_APP_CONFFILE "conf/FocusMotorCtrl.conf"
  *    #include "VisAOGUI_main.h"
  *    int main(int argc, char **argv)
  *    {
  *       return VisAOGUI_main(argc, argv);
  *    }
  *
*/

int TimeToDie;
//std::string global_app_name;

///Suppress annoying Qt library warnings and debug messages.
void myMessageOutput(QtMsgType type, const char *msg)
{
   switch (type) {
      case QtDebugMsg:
         break;
      case QtWarningMsg:
         break;
      case QtCriticalMsg:
         fprintf(stderr, "Critical: %s\n", msg);
         break;
      case QtFatalMsg:
         fprintf(stderr, "Fatal: %s\n", msg);
         abort();
   }
}

fifo_list * global_fifo_list;

int VisAOGUI_main( int argc, char **argv) 
{
   TimeToDie = 0;

   //Installs our warning suppressor.
   //qInstallMsgHandler(myMessageOutput);
   
   QApplication app(argc, argv);
   
   VISAO_APP_TYPE *c = NULL;
   
   try 
   {
      
      if (argc>1)
      {
         c = new VISAO_APP_TYPE(argc, argv);
      }
      else
      {
         #ifdef VISAO_APP_CONFFILE
            //Just for backwards compat.
            c = new VISAO_APP_TYPE(VISAO_APP_NAME, VISAO_APP_CONFFILE);
         #else
            std::string conffile = Utils::getConffile(VISAO_APP_NAME);
            c = new VISAO_APP_TYPE(VISAO_APP_NAME, conffile);
         #endif
            //c = new VISAO_APP_TYPE(VISAO_APP_NAME, VISAO_APP_CONFFILE);
      }

      c->show();
      
      return app.exec();

   } catch (AOException &e) 
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
   }
   
   delete c;
}

