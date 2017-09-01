/************************************************************
*    dioserver_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for the digital I/O server
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file dioserver_main.cpp
  * \author Jared R. Males
  * \brief The main program for the digital I/O server
  * 
  * Does not use VisAO_main because of the dio card setup, which is external
  * to the dioserver class.
  *
*/

//Here the order of include matters, apc464_dioserver.h must be first to avoid double define of WORD by Logger.h
#include "apc464_dioserver.h"
#include "dioserver.h"

int TimeToDie;
std::string global_app_name;

int main(int argc, char **argv)
{
   TimeToDie = 0;
      
   try
   {
      VisAO::dioserver * dios;
      
      apc464_info apc464;
      
      init_apc464_info(&apc464);
      
      if(argc > 1)
      {
         dios = new VisAO::dioserver(argc,argv);
      }
      else
      {
         std::string confdir = Utils::getConffile("dioserver");
         dios = new VisAO::dioserver("dioserver", confdir);
      }
      
      //Install the apc464 functions
      dios->diocard_info = (void *) &apc464;
      dios->diocard_init = &apc464_init;
      dios->diocard_write = &apc464_write;
      dios->diocard_read = &apc464_read;
      
      dios->Exec();
      
      delete dios;
      
      return 0;
      
   }
   catch(...)
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, "exception caught in dioserver");
      return -1;
   }
}

