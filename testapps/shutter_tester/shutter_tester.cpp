#include "shutter_tester.h"

namespace VisAO
{

shutter_tester::shutter_tester(int argc, char **argv) throw (AOException) : VisAOApp_standalone(argc, argv)
{
   initialize_shutter_tester();
}

shutter_tester::shutter_tester(std::string name, const std::string &conffile) throw (AOException) : VisAOApp_standalone(name, conffile)
{
   initialize_shutter_tester();
}

int shutter_tester::initialize_shutter_tester() throw (AOException)
{
   std::string pathtmp;
   std::string visao_root = getenv("VISAO_ROOT");
   
   testmode = testfreq;
   //testmode = testsim;
   
   threshold = .2;
   
   freq = 10.;
   
   dutycyc = 0.5;
   
   duration = 10;
   
   t_remaining = 0.;
   
   //Set up the com_paths
   try
   {
      pathtmp = (std::string)(ConfigDictionary())["com_path"];
   }
   catch(Config_File_Exception)
   {
      pathtmp = "fifos";
   }
   pathtmp = "/" + pathtmp + "/";
   
   
   com_path_local = visao_root + pathtmp + "shuttertester_com_local";
   _logger->log(Logger::LOG_LEV_INFO, "Set ShutterTester com_path_local: %s", com_path_local.c_str());
   
   
   setup_fifo_list(2);
   
   setup_baseApp();
   
   shutter_fifo = visao_root + "/fifos/shuttercontrol_com_auto";
   std::cout << shutter_fifo << std::endl;
   
   std::string sf_in = shutter_fifo + "_in";
   std::string sf_out = shutter_fifo + "_out";
   
   std::cout << sf_in << std::endl;
   
   std::cout << sf_out << std::endl;
   //set_fifo_list_channel(&fl, 0, RWBUFF_SZ, sf_out.c_str(), sf_in.c_str(),  &read_fifo_channel, (void *)this);
   
   set_fifo_list_channel(&fl, 0, RWBUFF_SZ, sf_out.c_str(), sf_in.c_str(),  0, 0);
   
   RunTest = 0;
   
   seq_no = 0;
   
   //Init the status board
   statusboard_shmemkey = 22000;
   if(create_statusboard(sizeof(shutter_tester_status_board)) != 0)
   {
      statusboard_shmemptr = 0;
      _logger->log(Logger::LOG_LEV_ERROR, "Could not create status board.");
   }
   else
   {
      VisAO::shutter_tester_status_board * stsb = (VisAO::shutter_tester_status_board *) statusboard_shmemptr;
      strncpy(stsb->appname, MyFullName().c_str(), 25);
      stsb->max_update_interval = pause_time;
   }
   
   simDeltaT = 0.001;
   testfile = getenv("VISAO_ROOT");
   testfile += "/bin/sims/data/longstrehl.txt";
   
   return 0;
   
}

std::string shutter_tester::local_command(std::string com)
{
   std::string rstr;
   char rchr[20];
   int rv;
   
   if(com == "start")
   {
      RunTest = 1;
      t_remaining = duration;
      return "starting\n";
   }
   if(com == "stop")
   {
      RunTest = 0;
      return "stopping\n";
      //goto shutter_tester_main_loop;
   }
   
   if(com == "remaining?")
   {
      if(!RunTest) return "0.000\n";
      
      snprintf(rchr, 20, "%0.3f\n", t_remaining);
      
      return rchr;
   }
   
   if(com == "testing?")
   {
      if(RunTest) return "1\n";
      else return "0\n";
   }
   
   if(com.find("freq") != std::string::npos)
   {
      if(com[4] == '?')
      {
         snprintf(rchr, 20, "%0.3f\n", freq);
         return rchr;
      }
      com.erase(0, 4);
      
      if(pthread_mutex_trylock(&my_mutex) == 0)
      {
         rv = set_freq(strtod(com.c_str(), 0));
         
         pthread_mutex_unlock(&my_mutex);
         
         if(rv) return "1\n";
                  else return "0\n";//success
      }
      else return "1\n";
   }
   
   if(com.find("testmode") != std::string::npos)
   {
      if(com[8] == '?')
      {
         snprintf(rchr, 20, "%i\n", testmode);
         return rchr;
      }
      com.erase(0, 8);
      
      if(pthread_mutex_trylock(&my_mutex) == 0)
      {
         rv = set_testmode(strtod(com.c_str(), 0));
         
         pthread_mutex_unlock(&my_mutex);
         
         if(rv) return "1\n";
                  else return "0\n";//success
      }
      else return "1\n";
   }
   
   if(com.find("testfile") != std::string::npos)
   {
      if(com[8] == '?')
      {
         return testfile + '\n';
      }
      com.erase(0, 9);
      
      if(pthread_mutex_trylock(&my_mutex) == 0)
      {
         rv = set_testfile(com);
         
         pthread_mutex_unlock(&my_mutex);
         
         if(rv) return "1\n";
      else return "0\n";//success
      }
      else return "1\n";
   }
   
   if(com.find("deltat") != std::string::npos)
   {
      if(com[6] == '?')
      {
         snprintf(rchr, 20, "%0.4f\n", simDeltaT);
         return rchr;
         
      }
      com.erase(0, 6);
      
      if(pthread_mutex_trylock(&my_mutex) == 0)
      {
         simDeltaT = strtod(com.c_str(),0);
         
         pthread_mutex_unlock(&my_mutex);
         
         return "0\n";
      }
      else return "1\n";
   }
   
   if(com.find("threshold") != std::string::npos)
   {
      if(com[9] == '?')
      {
         snprintf(rchr, 20, "%0.4f\n", threshold);
         return rchr;
      }
      com.erase(0, 9);
      
      if(pthread_mutex_trylock(&my_mutex) == 0)
      {
         rv = set_threshold(strtod(com.c_str(),0));
         
         pthread_mutex_unlock(&my_mutex);
         
         if(rv) return "1\n";
                  else return "0\n";//success
      }
      else return "1\n";
                  
   }
   
   if(com.find("dutycyc") != std::string::npos)
   {
      if(com[7] == '?')
      {
         snprintf(rchr, 20, "%0.3f\n", dutycyc);
         return rchr;
      }
      com.erase(0, 7);
      
      if(pthread_mutex_trylock(&my_mutex) == 0)
      {
         rv = set_dutycyc(strtod(com.c_str(),0));
         
         pthread_mutex_unlock(&my_mutex);
         
         if(rv) return "1\n";
                  else return "0\n";//success
      }
      else return "1\n";
                  
   }
   
   if(com.find("duration") != std::string::npos)
   {
      if(com[8] == '?')
      {
         snprintf(rchr, 20, "%0.3f\n", duration);
         return rchr;
      }
      com.erase(0, 8);
      
      if(pthread_mutex_trylock(&my_mutex) == 0)
      {
         rv = set_duration(strtod(com.c_str(),0));
         
         pthread_mutex_unlock(&my_mutex);
         
         if(rv) return "1\n";
                  else return "0\n";//success
      }
      else return "1\n";
                  
   }
   
   if(com.find("OPEN") != std::string::npos)
   {
      if(com[4] == '?')
      {
         snprintf(rchr, 20, "%i\n", act_state);
         return rchr;
      }
      
      if(pthread_mutex_trylock(&my_mutex) == 0)
      {
         write_fifo_channel(0, "AUTO", 5, &rstr);
         write_fifo_channel(0, "1", 2, &rstr);    //Need to understand this: why does writing again too fast not work?
         act_state = 1;
         write_fifo_channel(0, "~AUTO", 5, &rstr);
         pthread_mutex_unlock(&my_mutex);
         
         return "0\n"; //success
         
      }
      else return "1\n";
                  
   }
   
   if(com.find("CLOSE") != std::string::npos)
   {
      if(com[5] == '?')
      {
         snprintf(rchr, 20, "%i\n", act_state);
         return rchr;
      }
      
      if(pthread_mutex_trylock(&my_mutex) == 0)
      {
         write_fifo_channel(0, "AUTO", 5, &rstr);
         write_fifo_channel(0, "0", 2, &rstr);    //Need to understand this: why does writing again too fast not work?
         act_state = -1;
         write_fifo_channel(0, "~AUTO", 5, &rstr);
         pthread_mutex_unlock(&my_mutex);
         
         return "0\n"; //success
         
      }
      else return "1\n";
                  
   }
   
   if(rstr == "") return "UNKOWN COMMAND!: " + com + "\n";
                  
                  return rstr;
}

int shutter_tester::set_testmode(int sm)
{
   if(sm >=0 && sm < testmode_max)
   {
      testmode = sm;
      return 0;
   }
   else return -1;
}

int shutter_tester::set_testfile(std::string tf)
{
   testfile = tf;
   return 0;
}

int shutter_tester::set_threshold(double st)
{
   threshold = st;
   return 0;
}

int shutter_tester::set_freq(double f)
{
   if(f >= 0)
   {
      freq = f;
      return 0;
   }
   else return -1;
}

int shutter_tester::set_dutycyc(double dc)
{
   if(dc >= 0)
   {
      dutycyc = dc;
      return 0;
   }
   else return -1;
}

int shutter_tester::set_duration(double d)
{
   if(d >= 0)
   {
      duration = d;
      return 0;
   }
   else return -1;
}



int shutter_tester::Run()
{
   std::string resp;
   
   signal(SIGIO, SIG_IGN);
   signal(RTSIGIO, SIG_IGN);
   signal(RTSIGIGN, SIG_IGN);
   //Install the main thread handler
   if(install_sig_mainthread_catcher() != 0)
   {
      ERROR_REPORT("Error installing main thread catcher.");
      return -1;
   }
   
   //Startup the I/O signal handling thread
   if(start_signal_catcher(false) != 0)
   {
      ERROR_REPORT("Error starting signal catching thread.");
      return -1;
   }
   sleep(1);
   /**/
   //Now Block all I/O signals in this thread.
   if(block_sigio() != 0)
   {
      ERROR_REPORT("Error starting signal catching thread.");
      return -1;
   }
   
   LOG_INFO("starting up . . .");
   
   start_profiler();
   
   pthread_mutex_lock(&my_mutex);
   
   
   //write_fifo_channel(0, "AUTO", 5,&resp);
   
   //write_fifo_channel(0, "1",2,&resp);
   
   pthread_mutex_unlock(&my_mutex);
   
   act_state = 1;
   
   while(!TimeToDie)
   {
      pthread_mutex_lock(&signal_mutex);
      pthread_cond_wait(&signal_cond, &signal_mutex);
      pthread_mutex_unlock(&signal_mutex);
      
      if(RunTest) run_test();
                  pthread_cond_broadcast(&profile->thcond);
   }
   return 0;
}

int shutter_tester::run_test()
{
   if(testmode == testfreq) return freq_test();
                  if(testmode == testsim) return sim_test();
                  return 0;
}

#define timeval_to_secs(t) ((double) t.tv_sec + ((double)t.tv_usec)/1e6)
int shutter_tester::freq_test()
{
   double period, t0, t1;
   timeval tv0, tv1;
   std::string resp;
   
   char sm[6];
   char sn[2];
   char opmsg[8];
   opmsg[0] = '1';
   opmsg[1] = '\0';
         char clmsg[8];
         clmsg[0] = '0';
         clmsg[1] = '\0';
         
         sn[0] = 'x';
         sn[1] = 'S';
         
         std::vector<double> topen_sent;
         std::vector<double> topen_done;
         std::vector<double> topen_synch;
         
         std::vector<double> tshut_sent;
         std::vector<double> tshut_done;
         std::vector<double> tshut_synch;
         
         int vsz = (int) (duration*freq) + 1;
         
         topen_sent.resize(vsz);
         topen_done.resize(vsz);
         topen_synch.resize(vsz);
         
         tshut_sent.resize(vsz);
         tshut_done.resize(vsz);
         tshut_synch.resize(vsz);
         
         pthread_mutex_lock(&my_mutex);
         
         period = 1./freq;
         
         double dt_open = dutycyc*period;
         double t0open, t1open, tactopen;
         timeval tv0open, tv1open;
         
         double dt_closed = (1.-dutycyc)*period;
         double t0closed, t1closed, tactclosed;
         timeval tv0closed, tv1closed;
         
         timespec ts;
         
         
         
         std::cout.precision(6);
         int loops = 0;
         
         usleep(500000);//Starts open.  Do some pausing to make sure we get there.
         write_fifo_channel(0, "AUTO", 5, &resp);
         write_fifo_channel(0, "1", 1, &resp);
         act_state = 1;
         usleep(500000);
         
         gettimeofday(&tv0, 0);
         
         t0 = timeval_to_secs(tv0);
         t1open = t0open = t1 = t0;
         while(t1-t0 < duration && RunTest && !TimeToDie)
         {
            //gettimeofday(&tv1top,0);
            //t1top = timeval_to_secs(tv1top);
            
            
            //std::cout << "Loop time top: " << t1top - t1 << "\n";
            //Only do this loop while actual state is shut
            //usleep(1000);
            while(act_state == -1 && t1open-t0open < dt_open && RunTest)
            {
               //Test state - sync is probably disabled.
               if(t1open-t0open >= 0.006)
               {
                  act_state = 1;
                  topen_synch[loops - 1] = t1open-t0; //this open is actually from previous loop
               }
               //Get new time
               gettimeofday(&tv1open,0);
               t1open = timeval_to_secs(tv1open);
            }
            tactopen = t1open;
            
            
            //Here sleep for remaining time
            //usleep((int)(.5*(dt_open-(t1open-t0open))));
            while(t1open-t0open < dt_open && RunTest)
            {
               gettimeofday(&tv1open,0);
               t1open = timeval_to_secs(tv1open);
            }
            //std::cout << "OPEN " << t1open - t0open << " " << tactopen - t0open << "\n";
            //if(tactopen-t0open > .0005) break;
            
            
            gettimeofday(&tv0closed,0);
            t0closed = timeval_to_secs(tv0closed);
            //Issue Shut Command here
            //do_shut()
            
            load_seqmsg(&clmsg[2], sn, seq_no);
            seq_no++;
            
            clock_gettime(CLOCK_REALTIME, &ts);
            profile->logseqmsg(&clmsg[2], "stc1", &ts);
            //std::cout << "issuing shut\n";
            
            tshut_sent[loops] = get_curr_time()-t0;
            write_fifo_channel(0, &clmsg[0], 1, 0);//&resp);//, &clmsg[2]);
         
         
         
         clock_gettime(CLOCK_REALTIME, &ts);
         profile->logseqmsg(&clmsg[2], "stc2", &ts);
         
         gettimeofday(&tv0closed,0);
         t0closed = 0.5*(timeval_to_secs(tv0closed) + t0closed);
         t1closed = t0closed;
         
         tshut_done[loops] = timeval_to_secs(tv0closed)-t0;
         
         //Only do this loop while actual state is shut
         //usleep(1000);
         while(act_state == 1 && t1closed-t0closed < dt_closed && RunTest)
         {
            //Test state
            
            if(t1closed-t0closed >= 0.006)
            {
               act_state = -1;
               tshut_synch[loops] = t1closed-t0;
            }
            
            gettimeofday(&tv1closed,0);
            t1closed = timeval_to_secs(tv1closed);
         }
         tactclosed = t1closed;
         //Here sleep for remaining time
         //usleep((int)(.5*(dt_closed-(t1closed-t0closed))));
         while(t1closed-t0closed < dt_closed && RunTest)
         {
            gettimeofday(&tv1closed,0);
            t1closed = timeval_to_secs(tv1closed);
         }
         
         //std::cout << "Shut " << t1closed - t0closed << " " << tactclosed -  t0closed << "\n";
         
         
         gettimeofday(&tv0open,0);
         t0open = timeval_to_secs(tv0open);
         //Issue Open Command Here
         //do_open()
         
         load_seqmsg(&sm[0], sn, seq_no);
         seq_no++;
         
         
         clock_gettime(CLOCK_REALTIME, &ts);
         profile->logseqmsg(sm, "sto1", &ts);
         //write_fifo_channel(0, "1", 1, &resp, &sm[0]);
         write_fifo_channel(0, "1", 1, 0);//&resp);
         topen_sent[loops] = t0open - t0;
         
         clock_gettime(CLOCK_REALTIME, &ts);
         profile->logseqmsg(sm, "sto2", &ts);
         
         gettimeofday(&tv0open,0);
         t0open = 0.5*(timeval_to_secs(tv0open) + t0open);
         
         topen_done[loops] = timeval_to_secs(tv0open) - t0;
         
         t1open = t0open;
         
         gettimeofday(&tv1,0);
         t1 = timeval_to_secs(tv1);
         t_remaining = duration - (t1-t0);
         
         //std::cout << "Loop time bot-: " << t1 - t1top << "\n";
         pthread_cond_broadcast(&profile->thcond);
         loops++;
         }
         std::cout << "Elapsed " << t1 - t0 << " " << loops << "\n";
         
         RunTest = 0;
         t_remaining = 0.;
         
         pthread_cond_broadcast(&profile->thcond);
         while(get_curr_time() - t1 < .5)
         {
            usleep(500000);//Want to leave open.  Do some pausing to make sure we get there.
         }
         
         write_fifo_channel(0, "1", 1, &resp);
            act_state = 1;
            
            write_fifo_channel(0, "~AUTO", 5, &resp);
            
            pthread_mutex_unlock(&my_mutex);
            
            //Write file
            std::string fname;
            char fext[21];
            
            fname = getenv("VISAO_ROOT");
   fname += "/data/shutter_freq_";
   get_visao_filename(fext, &tv0);
   fname += fext;
   fname += ".txt";
   
   ofstream fout;
   fout.open(fname.c_str());
   fout.precision(10);
   
   for(int i=0;i<loops; i++)
   {
      fout << tshut_sent[i] << " " << tshut_done[i] << " " << tshut_synch[i] << " ";
      fout << topen_sent[i] << " " << topen_done[i] << " " << topen_synch[i] << "\n";
   }
   
   fout.close();
   
   
   //write_fifo_channel(0, "~LOCAL\n", 7);
   return 0;
}

int shutter_tester::sim_test()
{
   
   if(testfile == "") return -1;
      if(simDeltaT <= 0) return -1;
      if(threshold <= 0) return -1;
      pthread_mutex_lock(&my_mutex);
   
   //OK, here we go.
      
      int n_strehls;
      double * strehls;
      double t0, t1;
      int curr_i;
      std::string resp;
      
      std::cout << "Starting load\n";
      std::cout << testfile.c_str() << "\n";
      n_strehls = readcolumns(testfile.c_str(), " ,\t", '#', "d", &strehls);
      
      if(n_strehls <= 0) return -1;
                              
                              std::cout << "Loaded " << n_strehls << "\n";
      usleep(500000);//Starts open.  Do some pausing to make sure we get there.
      write_fifo_channel(0, "AUTO", 5, &resp);
      write_fifo_channel(0, "1", 1, &resp);
      act_state = 1;
      usleep(500000);
      
      t0 = get_curr_time();
      
      curr_i = 0;
      
      
      while(get_curr_time() - t0 < duration && curr_i < n_strehls && RunTest && !TimeToDie)
      {
         std::cout << curr_i << "\n";
         if(strehls[curr_i] >= threshold && act_state == -1)
         {
            std::cout << "Opening \n";// << get_curr_time() - t0 << "\n";
            write_fifo_channel(0, "1", 1, &resp);
            //std::cout << get_curr_time() - t0 << "\n";
            act_state = 1;
         }
         else if(strehls[curr_i] < threshold && act_state == 1)
         {
            std::cout << "Shutting \n";// << get_curr_time() - t0 << "\n";
            write_fifo_channel(0, "0", 1, &resp);
            //std::cout << get_curr_time() - t0 << "\n";
            act_state = -1;
         }
         
         t_remaining = duration - (get_curr_time()-t0);
         
         //usleep((int)(0.25*simDeltaT * 1e6));
         
         while((int)((get_curr_time()-t0)/simDeltaT) < curr_i+1)
         {
            usleep(1);
         }
         curr_i = (int)((get_curr_time()-t0)/simDeltaT);
      }
      std::cout << "Elapsed " << get_curr_time() - t0 << "\n";
      RunTest = 0;
      t_remaining = 0.;
      
      pthread_cond_broadcast(&profile->thcond);
      
      while(get_curr_time() - t1 < .5)
      {
         usleep(500000);//Want to leave open.  Do some pausing to make sure we get there.
      }
      
      write_fifo_channel(0, "1", 1, &resp);
      act_state = 1;
      write_fifo_channel(0, "~AUTO", 5, &resp);
      pthread_mutex_unlock(&my_mutex);
      
      return 0;
}









int shutter_tester::update_statusboard()
{
   
   if(statusboard_shmemptr)
   {
      VisAOApp_base::update_statusboard();
      
      VisAO::shutter_tester_status_board * stsb = (VisAO::shutter_tester_status_board *) statusboard_shmemptr;
      
      stsb->RunTest = RunTest;
      stsb->testmode = testmode;
      strncpy(stsb->testfile, testfile.c_str(), 255);
      stsb->threshold = threshold;
      stsb->freq = freq;
      stsb->dutycyc = dutycyc;
      stsb->duration = duration;
      
   }
   return 0;
}

} //namespace VisAO




