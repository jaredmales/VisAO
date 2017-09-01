extern "C" 
{
#include "hwlib/netseriallib.h"
#include "hwlib/joelib.h"
}

#include <iostream>
int debug;

int main()
{
      
   debug = 0;
   
   int result, stat;

   std::string _ccdNetAddr = "192.168.0.101";
   int _ccdNetPort = 4001;
   std::string control_filename = "/home/aosup/ccd39/2khz/arcetri2.4.close-con-com.bin";
   std::string pattern_filename = "/home/aosup/ccd39/2khz/arcetri2.4.close-pat-com.bin";

   //std::string control_filename = "/home/aosup/ccd39/2khz/arcetri3.3.close-con-com.bin";
   //std::string pattern_filename = "/home/aosup/ccd39/2khz/arcetri3.3.close-pat-com.bin";

   
   // Setup serial/network interface
   if (( result = InitJoeLink( (char *)_ccdNetAddr.c_str(), _ccdNetPort)) != 0)
   {
      std::cerr << "Network error\n";
      return -1;
   }


   if ( (stat = SendXmodemFile( "XMC 1", (char *) control_filename.c_str())) != 0)
      {
         std::cerr << "Xmodem upload returned error at " << __LINE__ << "\n";
         return stat;
      }
                
      if ( (stat = SendXmodemFile( "XMP 1", (char *) pattern_filename.c_str())) != 0)
      {
         std::cerr << "Xmodem upload returned error at " << __LINE__ << "\n";
         return stat;
      }

   return 0;
}


