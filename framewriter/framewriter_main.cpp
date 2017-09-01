/************************************************************
*    framewriter_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for the frame writer.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file framewriter_main.cpp
  * \author Jared R. Males
  * \brief The main program for the frame writer.
  *
*/

#include "framewriter.h"

#define VISAO_APP_TYPE VisAO::framewriter<short>
#define VISAO_APP_NAME "framewriter47"
//#define VISAO_APP_CONFFILE "conf/framewriter47.conf"
#include "VisAO_main.h"

int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}

/*
int TimeToDie;

int main(int argc, char **argv)
{
   TimeToDie = 0;
   
	try
	{
		VisAO::framewriter * fw;
   
		//fw = new framewriter;
		if(argc > 1)
		{
			fw = new VisAO::framewriter(argc,argv);
		}
		else
		{
			ostringstream oss;
         oss << getenv("VISAO_ROOT") << "conf/framewriter.conf";
			fw = new VisAO::framewriter(oss.str());
		}
		
   	fw->start();

		delete fw;
		
   	return 0;

	}
	catch(...)
	{
		std::cerr << "Exception caught\n";
		Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, "exception caught trying to start framewriter");
		return -1;
	}
}
*/
