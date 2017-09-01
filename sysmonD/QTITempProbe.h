

#ifndef __QTITempProbe_h__
#define __QTITempProbe_h__

#include "libvisao.h"

#include <fstream>

#include <iostream>
#include <fstream>
#include <cstdlib>



class QTITempProbe
{
   public:
      QTITempProbe();
      QTITempProbe(std::string dev_path);

      ~QTITempProbe();

      enum QTI_ERRORS{QTINO_ERROR, FIN_GOOD_ERROR, FOUT_GOOD_ERROR, DEV_NOT_OPEN_ERROR};
      
   protected:

      std::string devPath;
      
      bool devOpen;

      std::string serialNumber;

      std::ifstream fin;
      std::ofstream fout;

      int errNum;
      

      timer_t timerid;

       struct sigevent sev;
      struct itimerspec its;
      
   
      struct sigaction sa;

   public:
      
      void init();

      int setDevPath(std::string dev_path);

      int readSerialNumber();

      std::string getSerialNumber(){return serialNumber;}
      
      double getTemperature();

      int getErrNum();
};


#endif //__QTITempProbe_h__
