/************************************************************
*    framewriter_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for the frame writer of unsigned char data type.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file framewriter_main.cpp
  * \author Jared R. Males
  * \brief The main program for the frame writer of unsigned char data type.
  *
*/

#include "framewriter.h"

#define VISAO_APP_TYPE VisAO::framewriter<unsigned char>
#define VISAO_APP_NAME "framewriter39"
#include "VisAO_main.h"

int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}


