/************************************************************
*    fifoutils.c
*
* Author: Jared R. Males (jrmales@as.arizona.edu)
*
* Linux fifo utilities, definitions
*
************************************************************/

/** \file fifoutils.c
  * \author Jared R. Males
  * \brief Definitions for linux fifo utilities.
  * 
*/



#include "fifoutils.h"

#include "time.h"
#include "sys/time.h"

double get_currt()
{
   struct timespec tsp;
   clock_gettime(CLOCK_REALTIME, &tsp);

   return ((double)tsp.tv_sec) + ((double)tsp.tv_nsec)/1e9;
}

int init_fifo_channel(fifo_channel * fc)
{
   fc->fd_out = 0;
   fc->fd_in = 0;
   fc->infname = 0;
   fc->outfname = 0;
   fc->input_handler = 0;
   fc->server_response = 0;
   fc->sr_sz = 0;
   fc->auxdata = 0;
   fc->timeout = 0;
   fc->last_atime_out = 0;
   fc->last_atime_in = 0;
   return 0;
}

int setup_fifo_channel(fifo_channel *fc,size_t buffsz)
{
   if(fc->server_response) free(fc->server_response);
   
   fc->sr_sz = buffsz;
   
   fc->server_response = (char *)malloc(buffsz*sizeof(char));
   
   if(fc->server_response) return 0;
   else 
   {
      ERRMSG("server_response malloc failed in setup_fifo_channel.");
      return -1;
   }
}

int set_fifo_channel(fifo_channel *fc, const char *fin, const char *fout, int (*inp_hand)(fifo_channel *), void * adata)
{
   if(fc->infname) free(fc->infname);
   fc->infname = (char *)malloc((strlen(fin)+1)*sizeof(char));
   if(fc->infname == 0) 
   {
      ERRMSG("infname malloc failed in set_fifo_channel.");
      return -1;
   }
   
   if(fc->outfname) free(fc->outfname);
             fc->outfname = (char *)malloc((strlen(fout)+1)*sizeof(char));
   if(fc->outfname == 0) 
   {
      ERRMSG("outfname malloc failed in set_fifo_channel.");
      return -1;
   }
   
   strncpy(fc->infname, fin, (strlen(fin)+1));
   strncpy(fc->outfname, fout, (strlen(fout)+1));
   
   fc->input_handler = inp_hand;
   
   fc->auxdata = adata;
   
   return create_fifos(fc);
   
}

int create_fifos(fifo_channel *fc)
{
   //int i;
   //umask(0);

   errno = 0;
   if(mkfifo(fc->infname, S_IRUSR | S_IWUSR) !=0) // | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) != 0)
   {
      if(errno != EEXIST)
      {
         ERRMSG("Error creating input fifo.");
         return -1;
      }
   }
   errno = 0;
   if(mkfifo(fc->outfname, S_IRUSR | S_IWUSR) !=0 )//| S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) != 0)
   {
      if(errno != EEXIST)
      {
         ERRMSG("Error creating output fifo.");
         return -1;
      }
    }

   return 0;
}//int create_fifos(fifo_channel *fc)

int open_fifo_channel_in(fifo_channel *fc)
{
   int nbytes;
   char  rbuff[10];
   
   fc->fd_in = ropen_async(fc->infname, 0);

   if(fc->fd_in < 0)
   {
      ERRMSG("Error opening input fifo.");
      return -1;
   }

   //should lock.

   //Clear the fifo.
   if(fc->fd_in > 0)
   {
      nbytes = read(fc->fd_in, rbuff, 10);
      while(nbytes > 0) nbytes = read(fc->fd_in, rbuff, 10);
   }

   return 0;
}

int open_fifo_channel_in_nolock(fifo_channel *fc)
{
   int nbytes;
   char  rbuff[10];
   
   fc->fd_in = ropen_async(fc->infname, 1);

   if(fc->fd_in < 0)
   {
      ERRMSG("Error opening input fifo.");
      return -1;
   }

   if(fc->fd_in > 0)
   {
      nbytes = read(fc->fd_in, rbuff, 10);
      while(nbytes > 0) nbytes = read(fc->fd_in, rbuff, 10);
   }

   return 0;
}

int open_fifo_channel_out(fifo_channel *fc)
{
   
   fc->fd_out = wopen_nonblock(fc->outfname, 0);
   
   if(fc->fd_out < 0)
   {
      ERRMSG("Error opening output fifo.");
      return -1;
   }
   
   return 0;
}

int open_fifo_channel_out_nolock(fifo_channel *fc)
{
   
   fc->fd_out = wopen_nonblock(fc->outfname, 1);
   
   if(fc->fd_out < 0)
   {
      ERRMSG("Error opening output fifo.");
      return -1;
   }
   
   return 0;
}

int write_fifo_channel(fifo_channel *fc, const char * com, int comlen)
{
   int nbytes;

   if(fc->fd_out == 0)
   {
      ERRMSG("Fifo channel output not connected.");
      return -1;
   }

   if(comlen + 1 > RWBUFF_SZ)
   {
		ERRMSG("Output buffer to large.");
      return -1;
   }

   fc->server_response[0] = '\0';
   fc->seqmsg = fc->server_response;

   nbytes = write(fc->fd_out, com, comlen+1);
   
   return nbytes;
}

int write_fifo_channel_seqmsg(fifo_channel *fc, const char * com, int comlen, char * seqmsg)
{
   int nbytes;
   char wbuff[RWBUFF_SZ];

   if(fc->fd_out == 0)
   {
      ERRMSG("Fifo channel output not connected.");
      return -1;
   }

   if(comlen + 7 > RWBUFF_SZ)
   {
      ERRMSG("Output buffer to large.");
      return -1;
   }

   fc->server_response[0] = '\0';
   fc->seqmsg = fc->server_response;
  
   memcpy(wbuff, com, comlen+1);
   memcpy(wbuff+comlen+1, seqmsg, 6);
   
   nbytes = write(fc->fd_out, wbuff, comlen+7); // +7

   return nbytes;
}

int read_fifo_channel(fifo_channel * fc)
{
   int nbytes, totbytes;
   size_t slen;
   char rbuffer[RWBUFF_SZ];
   //testing vars:
   //char app[3];
   //uint32_t sn;

   if(fc->fd_in == 0)
   {
      ERRMSG("Fifo channel input not connected.");
      return -1;
   }

	totbytes = 0;
   
   errno = 0;
   nbytes = read(fc->fd_in, rbuffer, RWBUFF_SZ);

   //totbytes += nbytes;
   //rbuffer[nbytes+1] = '\0';
   
   
   while(nbytes > 0 && nbytes+totbytes < fc->sr_sz-1)
   {
		memcpy(fc->server_response + totbytes, rbuffer, nbytes);
      totbytes += nbytes;
      
      //snprintf(fc->server_response, fc->sr_sz, "%s%s", fc->server_response, rbuffer); //Probably a better way to do this?
      
      nbytes = read(fc->fd_in, rbuffer, RWBUFF_SZ);

      //rbuffer[nbytes+1] = '\0';
   }
   
   if(nbytes < 0 && errno != EAGAIN)
   {
         ERRMSG("Read error.");
   }
   
   if(totbytes + nbytes >= fc->sr_sz-1)
   {
      ERRMSG("input exceeded max server response size (fc->sr_sz).  fifo read truncated.");
   }

   if(totbytes > 0)
   {
      fc->server_response[totbytes] = '\0';
      
      slen = strlen(fc->server_response);
      //if(totbytes > slen) 
      fc->seqmsg = &(fc->server_response[slen+1]);
      //else fc->seqmsg = fc->server_response[slen];
   
      
    /* if(fc->seqmsg[0] != '\0')
      {
         printf("Sequence message detected\n");
         printf("%i\t%i\t|%s|\n", totbytes, (int) strlen(fc->server_response), fc->server_response);
         parse_seqmsg(app, &sn, fc->seqmsg);
         app[2] = '\0';
         printf("%s\t%i\n", app, sn);
      }*/
   }
   else fc->seqmsg = fc->server_response;
   
   return totbytes;
}

int check_if_in_locked(fifo_channel *fc)
{
   struct flock fl;
   fl.l_type = F_WRLCK; //check for an exclusive lock
   fl.l_whence = SEEK_SET;
   fl.l_start = 0;
   fl.l_len = 0;
   fl.l_pid = getpid();

   if(fcntl(fc->fd_out, F_GETLK, &fl) < 0) return 1;

   if(fl.l_type == F_UNLCK) return 0;
   else return 1;

   /*int pid;
   pid = fcntl(fc->fd_out, F_GETOWN);
   printf("%d\n", pid);
   
   return pid;*/
   
}


/******* fifo_list functions ************/

int init_fifo_list(fifo_list * fl)
{
   fl->nchan = 0;
   fl->fifo_ch = 0;
   FD_ZERO(&fl->fds);
   fl->maxfile = 0;
   fl->rq_sz = 1024;
   
   fl->RTSIGIO_overflow = 0;
   
   return 0;
}

int setup_fifo_list(fifo_list * fl, int nch)
{
   int i;

   fl->nchan = nch;

   fl->fifo_ch = (fifo_channel *)malloc(nch*sizeof(fifo_channel));

   if(fl->fifo_ch == 0)
   {
      ERRMSG("Error allocating fifo_list memory.");
      return -1;
   }

   for(i=0; i<nch; i++) init_fifo_channel(&fl->fifo_ch[i]);

   return 0;
}

int set_fifo_list_channel(fifo_list *fl, int nch, int buffsz, const char *fin, const char *fout, int (*inp_hand)(fifo_channel *), void *adata)
{
   if(setup_fifo_channel(&fl->fifo_ch[nch], buffsz) < 0) return -1;
   return set_fifo_channel(&fl->fifo_ch[nch], fin, fout, inp_hand, adata);
}

int connect_fifo_list(fifo_list *fl)
{
   int i;
   char errmsg[ERRMSG_SZ];

   for(i = 0; i < fl->nchan; i++)
   {
      if(open_fifo_channel_in(&fl->fifo_ch[i]) != 0)
      {
         snprintf(errmsg, ERRMSG_SZ, "Error opening input fifo channel %i (%s)", i, fl->fifo_ch[i].infname);
         ERRMSG(errmsg);
         return -1;
      }
      else
      {
         FD_SET(fl->fifo_ch[i].fd_in, &fl->fds);

         if(fl->fifo_ch[i].fd_in > fl->maxfile) fl->maxfile = fl->fifo_ch[i].fd_in;
      }

   }

   for(i = 0; i < fl->nchan; i++)
   {
      if(open_fifo_channel_out_nolock(&fl->fifo_ch[i]) != 0)
      {
         snprintf(errmsg, ERRMSG_SZ, "Error opening output fifo channel %i (%s)", i, fl->fifo_ch[i].outfname);
         ERRMSG(errmsg);
         return -1;
      }
   }

   return init_fifo_list_pending_reads(fl);

}

int connect_fifo_list_nolock(fifo_list *fl)
{
   int i;
   char errmsg[ERRMSG_SZ];
   //printf("4.4.1\n");
   for(i = 0; i < fl->nchan; i++)
   {
      //printf("4.4.2 %i\n", i);
      
      if(open_fifo_channel_in_nolock(&fl->fifo_ch[i]) != 0)
      {
         snprintf(errmsg, ERRMSG_SZ, "Error opening input fifo channel %i (%s)", i, fl->fifo_ch[i].infname);
         ERRMSG(errmsg);
         return -1;
      }
      else
      {
         //printf("4.4.3 %i\n", i);
         FD_SET(fl->fifo_ch[i].fd_in, &fl->fds);
         
         if(fl->fifo_ch[i].fd_in > fl->maxfile) fl->maxfile = fl->fifo_ch[i].fd_in;
      }
      
   }
   //printf("4.4.4\n");
   for(i = 0; i < fl->nchan; i++)
   {
      if(open_fifo_channel_out_nolock(&fl->fifo_ch[i]) != 0)
      {
         snprintf(errmsg, ERRMSG_SZ, "Error opening output fifo channel %i (%s)", i, fl->fifo_ch[i].outfname);
         ERRMSG(errmsg);
         return -1;
      }
   }
   //printf("4.4.5\n");
   return init_fifo_list_pending_reads(fl);
   
}

int init_fifo_list_pending_reads(fifo_list *fl)
{
   int i;

   fl->fd_to_fifo_ch_index = (int *) malloc((fl->maxfile+1)*sizeof(int));
   
   for(i = 0; i < fl->maxfile+1; i++)
   {
      fl->fd_to_fifo_ch_index[i] = 0;
   }

   for(i =0; i < fl->nchan; i++)
   {
      fl->fd_to_fifo_ch_index[fl->fifo_ch[i].fd_in] = i;
   }

   fl->tot_pending_reads = 0;

   fl->pending_reads = (int *) malloc(fl->nchan*sizeof(int));

   for(i=0;i<fl->nchan;i++)
   {
      fl->pending_reads[i] = 0;
   }

   fl->read_queue = (int *) malloc(fl->rq_sz*sizeof(int));

   for(i=0; i < fl->rq_sz; i++)
   {
      fl->read_queue[i] = 0;
   }
   fl->read_queue_pos = 0;
   fl->read_queue_nextpos = 0;

   return 0;
}

void catch_fifo_response_list(int signum __attribute__((unused)))
{
   int i;
   struct timeval tv;
   tv.tv_sec = 0;
   tv.tv_usec = 10;

   //printf("SIGIO Caught\n");
   
   select(global_fifo_list->maxfile+1, &global_fifo_list->fds, 0, 0, &tv);

   for(i=0;i<global_fifo_list->nchan;i++)
   {
      if(global_fifo_list->fifo_ch[i].fd_in > -1)
      {
         if(FD_ISSET(global_fifo_list->fifo_ch[i].fd_in, &global_fifo_list->fds)) 
         {
            global_fifo_list->fifo_ch[i].input_handler(&global_fifo_list->fifo_ch[i]);
         }
         else FD_SET(global_fifo_list->fifo_ch[i].fd_in, &global_fifo_list->fds); //reset for next time
      }
   }
   
   return;
}

int set_fifo_list_rtsig(fifo_list * fl)
{
   int i;
   
   for(i=0;i<fl->nchan; i++)
   {
      if(fl->fifo_ch[i].input_handler)
      {
         fcntl(fl->fifo_ch[i].fd_in, F_SETSIG, RTSIGIO);
      }
      else
      {
         fcntl(fl->fifo_ch[i].fd_in, F_SETSIG, RTSIGIGN);
      }
   }
   
   return 0;
}

void catch_fifo_pending_reads(int signum __attribute__((unused)), siginfo_t *siginf, void *ucont __attribute__((unused)))
{
   int fd;
   int idx;

   //double t = get_curr_time();
   //First check if this is an input available signal.
   //This is necessary since we have to open fifos as RW, a 
   //POLL_OUT SIGIO signal will be generated every time we clear one.
   if(siginf->si_code == POLL_IN)
   {
      global_fifo_list->tot_pending_reads++;
      
      fd = siginf->si_fd;
      idx = global_fifo_list->fd_to_fifo_ch_index[fd];
   
      global_fifo_list->pending_reads[idx]++;
 
      if(global_fifo_list->read_queue[global_fifo_list->read_queue_nextpos])
      {
         ERRMSG("Condition met.  RT Signal queue wraparound.");
         global_fifo_list->RTSIGIO_overflow = 1;
      }
      global_fifo_list->read_queue[global_fifo_list->read_queue_nextpos] = idx;
  
      global_fifo_list->read_queue_nextpos++;
  
      if(global_fifo_list->read_queue_nextpos >= global_fifo_list->rq_sz) global_fifo_list->read_queue_nextpos = 0;
   }

}

void catch_fifo_standard_sigio(int signum __attribute__((unused)))
{
   ERRMSG("Standard SIGIO caught.  RT Signal queue overflow.");   
   
   global_fifo_list->RTSIGIO_overflow = 1;   
}
   

int fifo_list_do_pending_read(fifo_list * fl)
{
   int i, rv;

   i = fl->read_queue[fl->read_queue_pos];

   fl->read_queue[fl->read_queue_pos] = 0; //This means it is okay to use this position again

   fl->tot_pending_reads--;

   fl->pending_reads[i]--;

   fl->read_queue_pos++;

   if(fl->read_queue_pos >= fl->rq_sz) fl->read_queue_pos = 0;

   //if(i == 4)
   //   printf("fldpr: %.10f\n", get_currt());
   
   if(fl->fifo_ch[i].input_handler)
   {
      rv = fl->fifo_ch[i].input_handler(&fl->fifo_ch[i]);
   }
   else
   {
      rv = 0;
   }
   
  return rv;
}

int ropen_async(char * fname, int nolock)
{
   int fd;
   errno = 0;
   char errmsg[ERRMSG_SZ];
   struct flock fl;
   
   errno = 0;
   fd = open(fname, O_RDWR| O_NONBLOCK);
   if(fd < 0)
   {
      snprintf(errmsg, ERRMSG_SZ, "Error opening fifo %s. Error = %s", fname, strerror(errno));
      ERRMSG(errmsg);
   }
   else
   {
      //Have to become owner to set O_ASYNC
      fcntl(fd, F_SETOWN, getpid());
      fcntl(fd, F_SETFL, 0);
      fcntl(fd, F_SETFL, O_ASYNC | O_RDWR | O_NONBLOCK);

      if(!nolock)
      {
         fl.l_type = F_WRLCK; //get an exclusive lock
         fl.l_whence = SEEK_SET;
         fl.l_start = 0;
         fl.l_len = 0;
         fl.l_pid = getpid();    
         errno = 0;
         if(fcntl(fd, F_SETLK, &fl) < 0)
         {
            snprintf(errmsg, ERRMSG_SZ, "Error getting exclusive lock on %s. Error = %s.  Closing file.", fname, strerror(errno));
            ERRMSG(errmsg);
            close(fd);
            fd = -1;
         }
      }
   }

   return fd;
}

int wopen_nonblock(char * fname, int nolock)
{
   int fd;
   char errmsg[ERRMSG_SZ];
   struct flock fl;

   errno = 0;
   fd = open(fname, O_RDWR | O_NONBLOCK);
   if(fd < 0)
   {
      snprintf(errmsg, ERRMSG_SZ, "Error opening fifo %s. Error = %s", fname, strerror(errno));
      ERRMSG(errmsg);
   }

   if(fd >=0 && !nolock)
   {
      fl.l_type = F_WRLCK; //get an exclusive lock
      fl.l_whence = SEEK_SET;
      fl.l_start = 0;
      fl.l_len = 0;
      fl.l_pid = getpid();

      //fcntl(fd, F_SETOWN, getpid());

      errno = 0;
      if(fcntl(fd, F_SETLK, &fl) < 0)
      {
         snprintf(errmsg, ERRMSG_SZ, "Error getting exclusive lock on %s. Error = %s.  Closing file.", fname, strerror(errno));
         ERRMSG(errmsg);

         close(fd);
         fd = -1;
      }
   }
   return fd;
}

