/************************************************************
*    ttyUSB.hpp
*
* Author: Jared R. Males (jrmales@as.arizona.edu)
*
* Find the details for USB serial devices
*
************************************************************/

/** \file ttyUSB.hpp
 * \author Jared R. Males
 * \brief Find the details for USB serial devices
 * 
 */

#ifndef __ttyUSB_hpp__
#define __ttyUSB_hpp__


#include <iostream>
#include <fstream>
#include <unistd.h>

#define USBbuffSz 1024

inline
int ttyUSBMajorNumber()
{
   std::ifstream fin;
   char line[USBbuffSz];
   
   fin.open("/proc/devices");
   
   int done = 0;
   char * pos = line;
   
   while(fin.good() && !done)
   {
      fin.getline(line, USBbuffSz);
      
      if(!fin.good())
      {
         fin.close();
         return -1;
      }
      
      pos = strstr(line, "ttyUSB");
      
      if( pos != NULL) done = 1;
   }
   fin.close();
   
   if( pos == NULL || !done)
   {
      return -1;
   }
   
   
   *pos = '\0';
   
   fin.close();
   
   return atoi(line);
}

inline
int ttyUSBMinorNumber( const char * vendor, const char * product )
{
   
   std::cerr << vendor << " " << product << "\n";
   std::ifstream fin;
   char line[USBbuffSz];
   
   std::string venStr = "vendor:";
   venStr += vendor;
   
   std::string prodStr = "product:";
   prodStr += product;
   
   fin.open("/proc/tty/driver/usbserial");

   char *posV = line, *posP = line;
   
   int done = 0;
   while(fin.good() && !done)
   {
      fin.getline(line, USBbuffSz);

      if(!fin.good())
      {
         fin.close();
         return -1;
      }
      
      posV = strstr(line, venStr.c_str());
      posP = strstr(line, prodStr.c_str());
      
      if(posV != NULL && posP != NULL) done = 1;
   }
   
   fin.close();
   
   if(posV == NULL || posP == NULL || !done)
   {
      return -1;
   }
   
   char * pc = strstr(line, ":");
   
   if(pc == NULL)
   {
      std::cerr << "Can not find the : in /proc/tty/driver/usbserial " << __FILE__ << " " << __LINE__ << "\n";
      return -1;
   }
   
   *pc = '\0';
   
   return atoi(line);
}
   
inline   
std::string ttyUSBdevName( const char * vendor, const char * product)
{
   std::string devName;
   
   char tmpFname[USBbuffSz];
   
   snprintf(tmpFname, USBbuffSz, "/tmp/ttyUSB_ls_%d", getpid());

   //Dump ttyUSB* info
   std::string lsstr = "ls -l /dev/ttyUSB* > ";
   lsstr += tmpFname; // /tmp/ttyUSB_ls";
   system(lsstr.c_str());
   
   //Get major and minor number
   int major = ttyUSBMajorNumber();
   int minor = ttyUSBMinorNumber(vendor, product);
   
   std::cerr << major << " " << minor << "\n";
   
   std::ifstream fin;
   char line[USBbuffSz];
   
   char mmStr[64];
   snprintf(mmStr, 64, "%d, %d", major, minor);
   
   fin.open(tmpFname);
   char * pos = line;
   int done = 0;
   
   while(fin.good() && ! done)
   {
      fin.getline(line, USBbuffSz);   
         
      if(!fin.good())
      {
         fin.close();
         return "";
      }
      
      pos = strstr(line, mmStr);
      
      if(pos != NULL) done = 1;
   }
   fin.close();
   
   if(pos == NULL || !done)
   {
      return devName;
   }
   
   pos = strstr(line, "/dev/");
   
   if(pos == NULL)
   {
      std::cerr << "ttyUSBdevName: can not find /dev/ " << __FILE__ << " " << __LINE__ << "\n";
      devName = "";
   }
   else
   {
      devName = pos;
   } 
      
   return devName;
}
   
inline
double ttyUSBdtime()
{
   timespec tsp;
   clock_gettime(CLOCK_REALTIME, &tsp);
      
   return ( (double)tsp.tv_sec + ((double) tsp.tv_nsec)/1e9);
}

inline
int fdWriteRead( char * buffRead, 
                 size_t buffReadSz, 
                 const char * buffWrite, 
                 size_t buffWriteSz, 
                 int fd,
                 int toWrite,
                 int toRead)
{
   int pv, rv, totrv;
   double t0;
   struct pollfd pfd;
   
   pfd.fd = fd;
   pfd.events = POLLOUT;
   
   t0 = ttyUSBdtime();
   
   pv = poll( &pfd, 1, toWrite); 
   if( pv == 0)
   {
      std::cerr << "ttyWriteRead: Time out waiting for write " << __FILE__ << " " << __LINE__ << "\n";  
      return -1;
   }
   else if(pv < 0 )
   {
      std::cerr << "ttyWriteRead: Error waiting for write " << __FILE__ << " " << __LINE__ << "\n";
      return -1;
   }
   
   rv = write(fd, buffWrite, buffWriteSz);
   totrv = rv;
   while(rv < buffWriteSz)
   {
      rv = write(fd, buffWrite, buffWriteSz);
      if(rv > 0) totrv += rv;
      
      if( (ttyUSBdtime()-t0)*1000 > toWrite )
      {
         std::cerr << "ttyWriteRead: timeout during write " << __FILE__ << " " << __LINE__ << "\n";
         totrv = -1;
         break;
      }
   }
   
   if(totrv < 0)
   {
      std::cerr << "ttyWriteRead: Error on write " << __FILE__ << " " << __LINE__ << "\n";
      return -1;
   }
   else if( totrv != buffWriteSz)
   {
      std::cerr << "ttyWriteRead: Did not write all chars " << __FILE__ << " " << __LINE__ << "\n";
      return -1;
   }
   
   

   t0 = ttyUSBdtime();
   
   pfd.events = POLLIN;
   pv = poll( &pfd, 1, toRead); 
   if( pv == 0)
   {
      std::cerr << "ttyWriteRead: Time out waiting for read " << __FILE__ << " " << __LINE__ << "\n";
      return 0;
   }
   else if(pv < 0 )
   {
      std::cerr << "ttyWriteRead: Error waiting for read " << __FILE__ << " " << __LINE__ << "\n";
      return -1;
   }
   
  
   
   rv = read(fd, buffRead, buffReadSz);
   totrv = rv;
   
   while(buffRead[totrv-1] != '\n')
   {  
      rv = read(fd, buffRead + totrv, buffReadSz - totrv);
      if(rv > 0) totrv += rv;
      
      if( (ttyUSBdtime()-t0)*1000 > toRead )
      {
         std::cerr << "ttyWriteRead: timeout during read " << __FILE__ << " " << __LINE__ << "\n";
         totrv = -1;
         break;
      }
   }
   
  
   
   if(totrv < 0 )
   {
      std::cerr << "ttyWriteRead: error on read " << __FILE__ << " " << __LINE__ << "\n";
   }
   return totrv;
   
   
}

inline
int fdWrite( const char * buffWrite, 
             size_t buffWriteSz, 
             int fd,
             int toWrite)
{
   int pv, rv, totrv;
   double t0;
   struct pollfd pfd;
   
   pfd.fd = fd;
   pfd.events = POLLOUT;
   
   t0 = ttyUSBdtime();
   
   pv = poll( &pfd, 1, toWrite); 
   if( pv == 0)
   {
      std::cerr << "ttyWriteRead: Time out waiting for write " << __FILE__ << " " << __LINE__ << "\n";
      return -1;
   }
   else if(pv < 0 )
   {
      std::cerr << "ttyWriteRead: Error waiting for write " << __FILE__ << " " << __LINE__ << "\n";
      return -1;
   }
   
   rv = write(fd, buffWrite, buffWriteSz);
   totrv = rv;
   while(rv < buffWriteSz)
   {
      rv = write(fd, buffWrite, buffWriteSz);
      if(rv > 0) totrv += rv;
      
      if( (ttyUSBdtime()-t0)*1000 > toWrite )
      {
         std::cerr << "ttyWriteRead: timeout during write " << __FILE__ << " " << __LINE__ << "\n";
         totrv = -1;
         break;
      }
   }
   
   if(totrv < 0)
   {
      std::cerr << "ttyWriteRead: Error on write " << __FILE__ << " " << __LINE__ << "\n";
      return -1;
   }
   else if( totrv != buffWriteSz)
   {
      std::cerr << "ttyWriteRead: Did not write all chars " << __FILE__ << " " << __LINE__ << "\n";
      return -1;
   }
   
   
   return totrv;
   
   
}

#endif // __ttyUSB_hpp__      
// int main()
// {
// 
// //    std::cout << ttyUSBMajorNumber() << "\n";
// //    std::cout << ttyUSBMinorNumber("0403", "6001") << "\n";
// //    std::cout << ttyUSBMinorNumber("1a72", "1014") << "\n";
// //    
// //    std::string devName;
//    std::cout << ttyUSBdevName("0403", "6001") << "\n";
//    std::cout << ttyUSBdevName("1a72", "1014") << "\n";
//    
//    return 0;
//    
// 
// 
// 
// }
