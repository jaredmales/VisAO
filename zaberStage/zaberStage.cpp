

#include "zaberStage.h"


double get_curr_time()
{
   struct timespec tsp;
   clock_gettime(CLOCK_REALTIME, &tsp);
   
   return ((double)tsp.tv_sec) + ((double)tsp.tv_nsec)/1e9;
}
   
void zaberStage::init()
{
   deviceNumber = 0;
   statusConnected = 0;
   statusHoming = -1;
   statusMoving = -1;
   statusHomed = -1;
   statusPosition = -1;
   
   commPause = 0.1;
   timeOut = 0.5;
  
   maxPosition = -1;
   
   pthread_mutex_init(&comm_mutex, NULL);
}

zaberStage::zaberStage()
{
   init();
   
}

zaberStage::zaberStage(std::string dname, uint8_t dnumber)
{
   init();
   
   deviceName = dname;
   deviceNumber = dnumber;
   
}

int zaberStage::connect()
{
   pthread_mutex_lock(&comm_mutex);
   if (zb_connect(&port, deviceName.c_str()) != Z_SUCCESS)
   {
      std::cerr << "Could not connect to device: " << deviceName << '\n';
      statusConnected = 0;
      
      pthread_mutex_unlock(&comm_mutex);
      return -1;
   }
   pthread_mutex_unlock(&comm_mutex);
            
   statusConnected = 1;
   
   //**** Turn off replies and lights
   int32_t answer;
   
   //First get status so we don't change the Home flag.
   if(sendCommand(53, 40, answer) < 0)
   {
      std::cerr << "1: Could not communicate with device: " << deviceName << '\n';
      statusConnected = 0;
      return -1;
   }
   
   sendCommand(40, 1 + (answer & 128) + 16384 + 32768);
   
   
   //Get max position
   sendCommand(53, 44, answer);
   maxPosition = answer;
  
   
   //**** Now check everything
   checkStatus();
   
   return 0;
}

int zaberStage::setTimeout(int ms)
{
   return zb_set_timeout(port, ms);
}

int zaberStage::sendCommand(uint8_t commandNumber, int32_t data, int32_t & answer, bool getResponse)
{
   int rv = -1;
   if(!statusConnected)
   {
      std::cerr << "Zaber stage " << deviceNumber << " not connected.\n";
      return -1;
   }
   
   zb_encode( destination, deviceNumber, commandNumber, data);
     
   pthread_mutex_lock(&comm_mutex);
   zb_send(port, destination);

   if(!getResponse)
   {
      pthread_mutex_unlock(&comm_mutex);
      return 0;
   }
   
   double t0 = get_curr_time();
   while(get_curr_time() - t0 < timeOut)
   {
      if( (rv = zb_receive(port, response)) > 0 ) break;
      usleep((int) (commPause*1e6));
   }
   pthread_mutex_unlock(&comm_mutex);
            
   zb_decode(&answer, response);

   
   if(rv != 6) return -1;
   else return 0;
}

int zaberStage::sendCommand(uint8_t commandNumber, int32_t data)
{
   int32_t answer;
   return sendCommand(commandNumber, data, answer, false);

}

int zaberStage::checkHomed()
{
   int32_t answer;
   sendCommand(53, 40, answer);
            
   statusHomed = ((answer & 128) > 0);
   return 0;
}

int zaberStage::checkPosition()
{
   if(!statusHomed) return -1;
   
   //Check if homed
   int32_t answer;
   
   int rv = sendCommand(60, 0, answer);
      
   if(rv == 0) statusPosition = answer;
   else statusPosition = -1;
   
   return 0;
}
   
int zaberStage::checkStatus()
{
   if(!statusConnected)
   {
      statusHomed = -1;
      statusPosition = -1;
      
      statusHoming = -1;
      statusMoving = -1;
      
      return 0;
   }
   
   int32_t answer;
   sendCommand(54, 0, answer);
   
   if(answer == 1) 
   {
      statusHoming = 1;
      statusMoving = 1;
   }
   else if(answer > 1)
   {
      statusHoming = 0;
      statusMoving = 1;
   }
   else
   {
      statusHoming = 0;
      statusMoving = 0;
   }
   
   checkHomed();      
   checkPosition();      
   
   return 0;
}

int zaberStage::home()
{
   sendCommand(1, 0);
 
   statusHoming = 1;
   return 0;
}

int zaberStage::moveAbs(int32_t pos)
{
   sendCommand(20, pos);
   statusMoving = 1;
   return 0;
}

int zaberStage::moveRel(int32_t dpos)
{
   sendCommand(21, dpos);
   statusMoving = 1;
   return 0;
}

int zaberStage::stop()
{      
   sendCommand(23, 0);
   return 0;
}
   
// int main()
// {
//    zaberStage zb("/dev/ttyUSB0", 1);
//    
//    zb.connect();
//    zb.setTimeout(100);
//    
//    std::cout << "Connected: " << zb.statusConnected << "\n";
//    std::cout << "Homing:   " << zb.statusHoming << "\n";
//    std::cout << "Moving:   " << zb.statusMoving << "\n";
//    std::cout << "Homed:    " << zb.statusHomed << "\n";
//    std::cout << "Position: " << zb.statusPosition << "\n";
//    
//    //return 0;
//    
//    zb.home();
// 
//    while(zb.statusHoming)
//    {
//       sleep(1);
//       zb.checkStatus();
//       std::cout << "Homing:   " << zb.statusHoming << "\n";
//       std::cout << "Moving:   " << zb.statusMoving << "\n";
//       std::cout << "Homed:    " << zb.statusHomed << "\n";
//       std::cout << "Position: " << zb.statusPosition << "\n";
//    }
//    
//    //return 0;
//    
//    zb.moveAbs(300000);
//    while(zb.statusMoving)
//    {
//       sleep(1);
//       zb.checkStatus();
//       std::cout << "Homing:   " << zb.statusHoming << "\n";
//       std::cout << "Moving:   " << zb.statusMoving << "\n";
//       std::cout << "Homed:    " << zb.statusHomed << "\n";
//       std::cout << "Position: " << zb.statusPosition << "\n";
//    }
//    
//    std::cout << "Homing:   " << zb.statusHoming << "\n";
//    std::cout << "Moving:   " << zb.statusMoving << "\n";
//    std::cout << "Homed:    " << zb.statusHomed << "\n";
//    std::cout << "Position: " << zb.statusPosition << "\n";
//       
// }
   



