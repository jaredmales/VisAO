/************************************************************
*    reconstructor_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for the reconstructor.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file reconstructor_main.cpp
  * \author Jared R. Males
  * \brief Main program for the reconstructor.  
  * 
*/

#include "reconstructor.h"


#define VISAO_APP_TYPE VisAO::reconstructor
#define VISAO_APP_NAME "reconstructor"
#include "VisAO_main.h"

int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}

/*


#include <sched.h>

int TimeToDie;

int main(int argc, char **argv)
{
   TimeToDie = 0;
   
	std::cout.precision(20);
   
	try
	{
		VisAO::ShutterControlDioclient * scd;
   
		if(argc > 1)
		{
			scd = new VisAO::ShutterControlDioclient(argc,argv);
		}
		else
		{
			ostringstream oss;
         oss << "conf/" <<  "ShutterControl.conf";
			scd = new VisAO::ShutterControlDioclient(oss.str());
		}
		
		
      
   	scd->Exec();

		delete scd;
		
   	return 0;

	}
	catch(...)
	{
		Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, "exception caught in ShutterControl");
		return -1;
	}
}
*/
