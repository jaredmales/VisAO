/************************************************************
 *    sharedim_stack.cpp
 *
 * Author: Jared R. Males (jrmales@email.arizona.edu)
 *
 * Definitions for the shared image circular buffer.
 *
 * Developed as part of the Magellan Adaptive Optics system.
 ************************************************************/

/** \file sharedim_stack.cpp
 * \author Jared R. Males
 * \brief Definitions for the shared image circular buffer.
 * 
 */

#include "sharedim_stack.h"

#ifndef ERROR_REPORT
#define ERROR_REPORT(er) if(global_error_report) (*global_error_report)(er,__FILE__,__LINE__);
#endif

#ifndef LOG_INFO
#define LOG_INFO(li) if(global_log_info) (*global_log_info)(li);
#endif

sharedim_stack::sharedim_stack()
{
   shmemptr = 0;
   shmemkey = -1;
   shmemid = 0;
   shmemsz = 0;
   
   header = 0;
   
   image_list = 0;
   
   next_last_image = -1;
}

int sharedim_stack::get_n_images()
{
   if(shmemptr > 0)
   {
      return header->n_images;
   }
   else return -1;
}

int sharedim_stack::set_max_n_images(int mni)
{
   if(shmemptr > 0)
   {
      if(header == 0) header = (sharedim_stack_header *) shmemptr;
      header->max_n_images = mni;
      
      //Calculate the offset of the first image
      image_list[0] = sizeof(sharedim_stack_header) + mni*sizeof(intptr_t);
      
      return 0;
   }
   else return -1;
}

int sharedim_stack::get_max_n_images()
{
   if(shmemptr > 0)
   {
      return header->max_n_images;
   }
   else return -1;
}

int sharedim_stack::get_last_image()
{
   if(shmemptr > 0)
   {
      return header->last_image;
   }
   else return -1;
}

sharedim * sharedim_stack::calc_im_pos(int imno)
{
   sharedim * previm;
   
   if(shmemptr > 0)
   {
      if(imno > header->n_images)
      {
         ERROR_REPORT("Can't calculate image offset without previous image.");
         return 0;
      }
      
      if(imno == 0) //this is easy
      {
         return (sharedim *)((intptr_t)header + image_list[0]);
      }
      
      //The previous image is set up already, so we use its image_list entry
      previm = (sharedim *)((intptr_t)header + image_list[imno-1]);
      sharedim * p0 = previm;

      size_t offset = (sizeof(sharedim) + (previm->nx*previm->ny*sizeof(IMDATA_TYPE)));
      previm = (sharedim *)((intptr_t) previm +   (size_t) offset);
      
      return previm;
   }
   else return 0; //can't do anything
}

int sharedim_stack::create_shm(key_t mkey, size_t sz)
{
   int rv;
   std::ostringstream oss;
   
   errno = 0;
   if((shmemid = shmget(mkey, sz, IPC_CREAT | 0666))<0)
   {
      shmemid = shmget(mkey, 1, 0666);
      
      //If it failed, try to remove the shmem block and recreate it.
      
      if(shmctl(shmemid, IPC_RMID, 0) < 0)
      {
         oss << "Could not remove shared memory with key " << mkey << ".";
         ERROR_REPORT(oss.str().c_str());
         return -1;
      }
      oss << "Removed shared memory with key " << mkey << ".";
      LOG_INFO(oss.str().c_str());
      oss.str("");
      
      if((shmemid = shmget(mkey, sz, IPC_CREAT | 0666))<0)
      {
         oss << "Could not create shared memory with key " << mkey << ".";
         ERROR_REPORT(oss.str().c_str());
         return -1;
      }
   }
   
   oss << "Shared memory created with key " << mkey << ".";
   LOG_INFO(oss.str().c_str());
   
   rv = attach_shm(mkey);
   //std::cout << sz << "\n";
   //std::cout << shmemsz << "\n";
   
   return rv;
}

int sharedim_stack::create_shm(key_t mkey, int nims, size_t sz)
{
   if(create_shm(mkey, sizeof(sharedim_stack_header) +nims*sizeof(intptr_t)+ sz) < 0)
   {
      return -1;
   }
   
   set_max_n_images(nims);
   header->last_image = -1;
   header->last_image_abs = -1;
   header->n_images = 0;
   
   return 0;
}

int sharedim_stack::attach_shm(key_t mkey)
{
   std::ostringstream oss;
   static int reported = 0;
   if(shmemid == 0)
   {
      if((shmemid = shmget(mkey, 0, 0666))<0)
      {
         if(!reported)
         {
            oss << "Could not get shared memory with key " << mkey;
            ERROR_REPORT(oss.str().c_str());
         }
         reported = 1;
         return -1;
      }
   }
   
   if ((shmemptr = shmat(shmemid, 0, 0)) == (char *) -1)
   {
      oss << "Could not attach shared memory with key " << mkey;
      ERROR_REPORT(oss.str().c_str());
      return -1;
   }
   
   if (shmctl(shmemid, IPC_STAT, &shmstats) < 0)
   {
      oss << "Could not get shared memory stats with key " << mkey;
      ERROR_REPORT(oss.str().c_str());
      return -1;
   }
   
   shmemkey = mkey;
   shmemsz = shmstats.shm_segsz;
   
   oss << "Attached to shared memory with key " << mkey << " of size: " << shmemsz;
   LOG_INFO(oss.str().c_str());
   
   header = (sharedim_stack_header *) shmemptr;
   
   image_list = (intptr_t *) ((intptr_t) shmemptr + sizeof(sharedim_stack_header));
   
   return 0;
}

sharedim * sharedim_stack::set_next_image(int nx, int ny)
{
   sharedim * nextim;
   sharedim * nextnextim;
   size_t offset;
   
   if(!shmemptr) return 0;
   
   //First see if we need to even check anything, or if we need to just wrap around
   if(header->last_image + 1 < header->max_n_images)
   {
      nextim = calc_im_pos((header->last_image) + 1);
      if(nextim == 0)
      {
         std::cerr << "Error in calc_im_pos.  Last image " << " " <<header->last_image<<  std::endl;
         return 0;
      }
      
      offset =  sizeof(sharedim) + ((intptr_t)nx)*((intptr_t)ny)*sizeof(IMDATA_TYPE);
      //previm = (sharedim *)((intptr_t) previm +   (size_t) offset);
      
      
      nextnextim = (sharedim *)((intptr_t)nextim + offset);
      
      /*      std::cout << sizeof(sharedim) << "\n";
       *     std::cout << sizeof(IMDATA_TYPE) << "\n";
       *     std::cout << nx << "\n";
       *     std::cout << ny << "\n";
       *     std::cout << shmemsz  << "\n";
       *     std::cout << (intptr_t) nextnextim << "\n";
       *     std::cout << (intptr_t) shmemptr << "\n";
       *     std::cout << (intptr_t)nextnextim - (intptr_t) shmemptr<< "\n";
       */
      //Check if we'll exceed the shared memory size, wrap around if so.
      if( (intptr_t)nextnextim -  (intptr_t) shmemptr  > (intptr_t)(shmemsz-1))
      {
         std::cerr << "Wrapping " << (header->last_image) + 1 << " " << (intptr_t)nextnextim -  (intptr_t) shmemptr << "\n";
         next_last_image = 0;
         nextim = (sharedim *)((intptr_t)header + image_list[0]);
         //We also need to reset max_n_images
         //header->max_n_images = header->last_image;
      }
      else
      {
         //std::cerr << "B\n";
         //Nope, just use the next available image
         next_last_image = header->last_image + 1;
         //if(header->n_images <= next_last_image) header->n_images++;
         image_list[next_last_image] = (intptr_t)nextim-(intptr_t)header;
      }
   }
   else //Just wrapping around
   {
      //std::cerr << "C\n";
      nextim = (sharedim *)((intptr_t)header + (intptr_t)image_list[0]);
      next_last_image = 0;
   }
   
   //Now fill in the structure
   nextim->nx = nx;
   nextim->ny = ny;
   nextim->imdata = (IMDATA_TYPE *) ((intptr_t)nextim+sizeof(sharedim));   //this pointer is only valid in the calling process
   
   return nextim;
}

int sharedim_stack::enable_next_image()
{
   header->last_image = next_last_image;
   if(header->n_images <= next_last_image) header->n_images++;
   header->last_image_abs++;
   return 0;
}

sharedim sharedim_stack::get_image(int imno)
{
   sharedim *simraw, sim;
   
   //Initialize the sharedim struct (should have an initializer)
   sim.nx =0;
   sim.ny =0;
   sim.depth = 0;
   sim.imdata = 0;
   
   //std::cout << "get_image parms:" << imno << " " << header->n_images << " " << header->max_n_images << "\n";
   if(imno < header->n_images)
   {
      simraw = (sharedim *)((intptr_t) header + (intptr_t) image_list[imno]);
      
      //std::cout << "nx " << simraw->nx << "\n";
      
      sim.nx = simraw->nx;
      sim.ny = simraw->ny;
      sim.depth = simraw->depth;
      sim.frameNo = simraw->frameNo;
      sim.frame_time = simraw->frame_time;
      sim.imdata = (IMDATA_TYPE *) ((intptr_t)simraw + sizeof(sharedim));
   }
   //else std::cout << "what the f?\n";
   return sim;
   
}
