

#include "QTITempProbe.h"

//int qtitimeout;

// double get_curr_time()
// {
//    struct timespec tsp;
//    clock_gettime(CLOCK_REALTIME, &tsp);
//    
//    return ((double)tsp.tv_sec) + ((double)tsp.tv_nsec)/1e9;
// }


sig_atomic_t timed_out;

static void timeout_handler(int sig, siginfo_t *si, void *uc)
{
   timed_out = 1;
}


QTITempProbe::QTITempProbe()
{
   init();
}




QTITempProbe::QTITempProbe(std::string dev_path)
{
   init();
   setDevPath(dev_path);
}

QTITempProbe::~QTITempProbe()
{
   if(devOpen)
   {
      fin.close();
      fout.close();
   }
}

void QTITempProbe::init()
{
   devOpen = 0;
   errNum = 0;

   long long freq_nanosecs;

   sa.sa_flags = SA_SIGINFO;
   sa.sa_sigaction = timeout_handler;
   sigemptyset(&sa.sa_mask);
   //sigaction(RTSIGTIMEOUT, &sa, NULL);

   
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = RTSIGTIMEOUT;
    sev.sigev_value.sival_ptr = &timerid;

   
   freq_nanosecs = (long long) 2.*1e9;
   its.it_value.tv_sec = freq_nanosecs/1000000000;
   its.it_value.tv_nsec = freq_nanosecs % 1000000000;
   its.it_interval.tv_sec = 0;
   its.it_interval.tv_nsec = 0;

   timer_create(CLOCK_REALTIME, &sev, &timerid);

}

int QTITempProbe::setDevPath(std::string dev_path)
{
   devPath = dev_path;

   fin.open(devPath.c_str());

   if(!fin.good())
   {
      std::cerr << "Error opening device path " << dev_path << " for reading.\n";
      devPath = "";
      devOpen = 0;
      errNum = FIN_GOOD_ERROR;
      return -1;
   }

   fout.open(devPath.c_str());
   
   if(!fout.good())
   {
      std::cerr << "Error opening device path " << dev_path << " for writing.\n";
      devPath = "";
      devOpen = 0;
      errNum = FOUT_GOOD_ERROR;
      return -1;
   }

   devOpen = 1;

   errNum = 0;

   //std::cerr << "Opened\n";
   return 0;
}

int QTITempProbe::readSerialNumber()
{
   char details[256];
   
   //qtitimeout = 0;

   if(!devOpen)
   {
      std::cerr << "Device not open.  Cannot get serial number.\n";
      errNum = DEV_NOT_OPEN_ERROR;

      return -1;
   }

   if(!fout.good())
   {
      std::cerr << "Device " << devPath << " output is not good.\n";
      errNum = FOUT_GOOD_ERROR;
      return -1;
   }

   

   std::cout << "1.0" << std::endl;

   //First stop any streaming and clear the buffer.
   
   //fout << "x" << std::endl;
   fout.put('x');
   fout.flush();

   //if(qtitimeout) return -1;
   std::cout << "1.1" << std::endl;
   usleep(50000);
   std::cout << "1.2" << std::endl;
   fin.readsome(details, 256);
   std::cout << "1.3" << std::endl;
   while(fin.gcount() > 0 /*&& !qtitimeout*/) fin.readsome(details, 256);

   //if(qtitimeout) return -1;

   std::cout << "1.4" << std::endl;
   /*details[0] = 0;
   fin.readsome(details, 256);

   while(details[0] != 0)std::cout << "GT 2.0\n";
   {
      std::cout << details << "--->\n";
      details[0] = 0;
      fin.readsome(details, 256); 
   }*/

   //fout << "3" << std::endl;
   fout.put('3');
   std::cout << "1.1" << std::endl;
   fout.flush();

   std::cout << "1.5" << std::endl;

   if(!fin.good())
   {
      std::cerr << "Device " << devPath << " input is not good.\n";
      errNum = FIN_GOOD_ERROR;
      return -1;
   }

   details[0] = 0;

   std::string line;
   int isser;
   std::cout << "1.6" << std::endl;
   //sleep(1);
   //exit(0);
   for(int i=0;i<20;i++)
   {
      fin.getline(details, 256);

      //if(qtitimeout) return -1;
      
      line = details;
      int srsn = line.find_first_of("\r\n");
      if(srsn > -1) line[srsn] = '\0';

      std::cout << i << "+=" << line << " ";

      isser = line.find("Ser.", 0);
      std::cout << isser << "\n";
      if(isser > -1) break;
   }
   ///\todo handle isser == -1 here

   std::cout << "Found: " << line << "\n";
   //fin.getline(details, 256);
   //std::cout << "\n------------\n" << details << "\n---------------\n";

   //if(qtitimeout) return -1;

   //Kill the line feed.
   details[fin.gcount()-1] = 0;
   
   serialNumber = details+7;

   std::cout << "|<|" << serialNumber << "|>|\n";
   
   //fin.getline(details, 256);
   //std::cout  << details << "\n";
   //if(qtitimeout) return -1;

   //usleep(250000);

   //Flush last > char and anything else
   fin.readsome(details, 256);
   while(fin.gcount() > 0 /*&& !qtitimeout*/) fin.readsome(details, 256);

   //if(qtitimeout) return -1;

   errNum = 0;

   fin.close();
   fout.close();

   return 0;
   
//    catch(int e)
//    {
//       std::cout << "Caught\n";
//       return -1;
//    }
}


double QTITempProbe::getTemperature()
{
   sigaction(RTSIGTIMEOUT, &sa, NULL);

   timed_out = 0;
   timer_settime(timerid, 0, &its, 0);

   char ts[256];
   double lt0, ldt;
   extern int TimeToDie;

   FILE *fo, *fi;

   //fout.open(devPath.c_str());
   fo = fopen(devPath.c_str(),"w");

   if(!devOpen)
   {
      std::cerr << "Device not open.  Cannot get temperature.\n";
      errNum = DEV_NOT_OPEN_ERROR;
      
      return -1;
   }

   
   ts[0] = 0;

   //just in case
   std::cout << "GT 1.0" << std::endl;

   //fin.sync();
   //fin.readsome(ts, 256);

//    fout << "x" << std::endl;
//    usleep(100000);
//    fin.readsome(ts, 256);
//    while(fin.gcount() > 0) fin.readsome(ts, 256);

   fi = fopen(devPath.c_str(), "r");
   fsync(fileno(fi));

   std::cout << "GT 2.0" << std::endl;
   //Must flush!
   //fout << "2" << std::endl;
   //fout.put('2');
   putc('2',fo);
   std::cout << "GT 2.1" << std::endl;
   fflush(fo);
   //fout.flush();

   fclose(fo);

   //if(timed_out) return -1000;

   //fout.close();
   //fin.open(devPath.c_str());
   

   if(timed_out) 
   {
      //return -1000;
      fsync(fileno(fi));
      timed_out = 0;
   }
   std::cout << "GT 3.0"  << std::endl;

   //fin.readsome(ts, 256);
   //std::cout << "ts[0]= " << (int) ts[0] << "\n";
   fgets(ts, 256, fi);
   if(timed_out) return -1000;
   std::cout << ts << std::endl;
   std::cout << "GT 4.0" << std::endl;

   lt0 = get_curr_time();
   ldt = 0.;
   while((ts[0] == 0 || strlen(ts) < 6) && ldt < 3.0 && !timed_out)
   {
      fsync(fileno(fi));
      fgets(ts, 256, fi);
      std::cout << ts << std::endl;
      //fin.readsome(ts, 256);
      ldt = lt0 - get_curr_time();
      //std::cout << "ts[0]= " << (int) ts[0] << "\n";
   }
   if(timed_out) return -1000;
   if(ldt >= 3.)
   {
      std::cerr << "get temperature timeout\n";
      return -100000;
   }

   if(ts[0] == '>') ts[0] = ' ';
   //std::cout << "ts = " << ts << "|" << strlen(ts) << "\n";

   //fin.close();
   fclose(fi);

   std::cout << "===>" << strtod(ts, 0) << std::endl;
   return strtod(ts, 0);

}





// int main(int argc, char *argv[])
// {
//    QTITempProbe qti;
//    
//    if(argc < 2)
//       qti.setDevPath("/dev/ttyACM0");
//    else
//       qti.setDevPath(argv[1]);
// 
//    qti.readSerialNumber();
// 
//    double t0 = get_curr_time();
//    std::cout.precision(20);
//    std::cout << "#" << qti.getSerialNumber() << "\n";
//    std::cout << "#" << t0 << "\n";
//    std::cout.precision(5);
//    for(int i=0;i<100000;i++)
//    {
// 
//       std::cout << get_curr_time() - t0 << " " << qti.getTemperature() << std::endl;
//       sleep(2);
//    }
//    
//    return 0;
// }

