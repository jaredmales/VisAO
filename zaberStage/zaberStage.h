


#ifndef __zaberStage_h__
#define __zaberStage_h__

//#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>

#include <string>
#include <iostream>

#include "time.h"
#include "sys/time.h"

#include "unistd.h"


#include "zaber/zb_serial.h"

class zaberStage
{
   void init();
   
public:
   std::string deviceName;
   uint8_t deviceNumber;
   
   z_port port;
   
   uint8_t destination[6];
   uint8_t response[6];
   

   int statusConnected;
   int statusHoming;
   int statusMoving;
   
   int statusHomed;
   int32_t statusPosition;
   
   int32_t maxPosition;
   
   double commPause;
   double timeOut;
   
   ///Mutex used for communications
   pthread_mutex_t comm_mutex;
   
   zaberStage();
   
   zaberStage(std::string dname, uint8_t dnumber);

   int connect();

   int setTimeout(int ms);

   int sendCommand(uint8_t commandNumber, int32_t data, int32_t & answer, bool getResponse = true);
   
   int sendCommand(uint8_t commandNumber, int32_t data);
   
   int checkHomed();
   
   int checkPosition();
      
   int checkStatus();
   
   int home();
   
   int moveAbs(int32_t pos);
   
   int moveRel(int32_t dpos);
   
   int stop();
};


   
#endif // __zaberStage_h__


