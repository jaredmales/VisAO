/************************************************************
*    libvisaoc.c
*
* Author: Jared R. Males (jrmales@as.arizona.edu)
*
* VisAO software utilitites, definitions
*
************************************************************/

/** \file libvisaoc.c
  * \author Jared R. Males
  * \brief VisAO software utilitites, definitions in c
  * 
*/

#include "libvisao.h"

void fifo_error_message(const char *msg, const char *file, int line)
{
   (*global_error_report)(msg, file, line);
}

int get_dio_fnames(char *fout, char *fin, char *fbase, int ch)
{
   snprintf(fout, MAX_FNAME_SZ, "%s_out_%02i", fbase, ch);
   snprintf(fin, MAX_FNAME_SZ, "%s_in_%02i", fbase, ch);
   return 0;
}

double ts_to_curr_time(struct timespec *tsp)
{
   return ((double)tsp->tv_sec) + ((double)tsp->tv_nsec)/1e9;
}

double tv_to_curr_time(struct timeval *tvp)
{
   return ((double)tvp->tv_sec) + ((double)tvp->tv_usec)/1e6;
}

double get_curr_time()
{
   struct timespec tsp;
   clock_gettime(CLOCK_REALTIME, &tsp);
   
   return ((double)tsp.tv_sec) + ((double)tsp.tv_nsec)/1e9;
}


int create_shmem(int * shmemid, key_t mkey, size_t sz)
{
   char oss[256];
   static int reported = 0;
   
   if((*shmemid = shmget(mkey, sz, IPC_CREAT | 0666))<0)
   {
      //If it failed, try to remove the shmem block and recreate it.
      *shmemid  = shmget(mkey, 1, 0666);
      if(shmctl(*shmemid, IPC_RMID, 0) < 0)
      {
         if(!reported)
         {
            snprintf(oss, 256, "Could not remove shared memory with key %i.", mkey);
            global_error_report(oss, __FILE__, __LINE__);
         }
         reported = 1;
         return -1;
      }
      snprintf(oss, 256, "Removed shared memory with key %i.", mkey);
      global_log_info(oss);
    
      
      if((*shmemid = shmget(mkey, sz, IPC_CREAT | 0666))<0)
      {
         if(!reported)
         {
            snprintf(oss, 256, "Could not create shared memory with key %i.", mkey);
            global_error_report(oss, __FILE__, __LINE__);
         }
         reported = 1;
         return -1;
      }
   }
   
   snprintf(oss, 256, "Shared memory created with key %i.", mkey);
   global_log_info(oss);
   
   return 0;
}

void * attach_shm(size_t *sz,  key_t mkey, int shmemid)
{
   char oss[256];
   void * shmemptr;
   static int reported = 0;
   
   struct shmid_ds shmstats;
   
   if(shmemid == 0)
   {
      if((shmemid = shmget(mkey, 0, 0666))<0)
      {
         if(!reported)
         {
            snprintf(oss, 256, "Could not get shared memory with key %i.", mkey);
            global_error_report(oss, __FILE__, __LINE__);
         }
         reported = 1;
         return 0;
      }
   }
   
   if ((shmemptr = shmat(shmemid, 0, 0)) == (char *) -1) 
   {
      if(!reported)
      {
        snprintf(oss, 256, "Could not attach shared memory with key %i.", mkey);
        global_error_report(oss, __FILE__, __LINE__);
      }
      reported = 1;
      return 0;
   }
   
   if (shmctl(shmemid, IPC_STAT, &shmstats) < 0)
   {
      if(!reported)
      {
         snprintf(oss, 256, "Could not get shared memory stats with key %i.", mkey);
         global_error_report(oss, __FILE__, __LINE__);
      }
      reported = 1;
      return 0;
   }

   *sz = shmstats.shm_segsz;
   
   snprintf(oss, 256, "Attached to shared memory with key %i of size %zu.", mkey , *sz);
   global_log_info(oss);

   return shmemptr;
}

double ComputeFramerate(double delay_base, double delay_inc, int rep)
{
   return(1.0e6/ (delay_base + delay_inc * rep));
}

int ComputeRepsFrameRate(double delay_base, double delay_inc, double fr)
{
   int rep;
   
   rep = ((1.0e6/fr) - delay_base)/delay_inc;
      
   return rep;
}

int ComputeRepsExpTime(double delay_base, double delay_inc, double et)
{
   return ComputeRepsFrameRate(delay_base, delay_inc, 1./et);
}

int sigproof_sleep(double secs, int *dienow)
{
   double dt, t0, trem;
   int dienow_tmp = 1;
   
   if(dienow == 0) dienow = &dienow_tmp;
   
   t0 = get_curr_time();
   
   while((dt = get_curr_time()-t0 < secs) && !(*dienow))
   {
      trem = 0.75*(secs-dt);
      if(trem >= 1.) sleep((int)trem);
      else usleep((int)(trem*1e6));
   }
   
   return 0;
}