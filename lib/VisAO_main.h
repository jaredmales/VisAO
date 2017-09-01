/************************************************************
*    VisAO_main.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for a VisAO application.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file VisAO_main.h
  * \author Jared R. Males
  * \brief The main program for a VisAO application.
  * 
  * Define VISAO_APP_TYPE and VISAO_APP_CONFFILE before including this in your main program.
  * Then just call return VisAO_main(argc, argv) as the only line in main()
  * e.g:
  *    #define VISAO_APP_TYPE VisAO::FocusMotorCtrl
  *    #define VISAO_APP_CONFFILE "conf/FocusMotorCtrl.conf"
  *    #include "VisAO_main.h"
  *    int main(int argc, char **argv)
  *    {
  *       return VisAO_main(argc, argv);
  *    }
  *
*/


//Globals
int TimeToDie;
int debug;
//std::string global_app_name;

fifo_list * global_fifo_list;

int VisAO_main( int argc, char **argv) 
{
   TimeToDie = 0;

   #ifdef _debug
      debug = 1;
   #else
      debug = 0;
   #endif

	try 
   {

      VISAO_APP_TYPE *c;

      if (argc>1)
      {
         c = new VISAO_APP_TYPE( argc, argv);
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
      }
   
      std::cout << "Execing\n";
      c->Exec();

      delete c;
      
      return 0;
   } 
   catch (AOException &e) 
   {
      std::cout << "Exception: " << e.what() << std::endl;

      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
      return -1;
   }
}


