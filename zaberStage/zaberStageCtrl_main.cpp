/************************************************************
*    zaberStageCtrl_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for the zaberStage controller.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file zaberStageCtrl_main.cpp
  * \author Jared R. Males
  * \brief The main program for the zaber stage controller.
  *
  */

#include "zaberStageCtrl.h"

#define VISAO_APP_TYPE VisAO::zaberStageCtrl
#define VISAO_APP_NAME "fieldstop"
#include "VisAO_main.h"

int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}
