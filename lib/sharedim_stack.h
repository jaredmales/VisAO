/************************************************************
 *    sharedim_stack.h
 *
 * Author: Jared R. Males (jrmales@email.arizona.edu)
 *
 * Declarations for the shared image circular buffer.
 *
 * Developed as part of the Magellan Adaptive Optics system.
 ************************************************************/

/** \file sharedim_stack.h
 * \author Jared R. Males
 * \brief Declarations for the shared image circular buffer.
 * 
 */


#ifndef __SHAREDIM_STACK_H__
#define __SHAREDIM_STACK_H__

#define IM0_OFFSET (sizeof(sharedim_stack_header) + header->n_images*sizeof(int))
#define IMD_OFFSET (2*sizeof(int)+sizeof(int *))

#include <sys/shm.h>
#include <sys/time.h>

#include <iostream>
#include <sstream>

#include "libvisao.h"

//#define IMDATA_TYPE short


/************************************************/
/* The sharedim_stack layout:
 *  Header:
 *   int max_n_images
 *   int n_images
 *   int last_image
 *   int last_image_abs
 *  Image offset list:
 *        int offset 0   <This is the memory offset from position 0 to the position of the first image>
 *        int offset 1
 *         . . .
 *        int offset max_n_images-1
 *  Images (struct sharedim):
 *        int nx
 *        int ny
 *        int depth
 *        int frameNo
 *        timeval frame_time
 *        IMDATA_TYPE * imdata
 *        IMDATA_TYPE imdata[0]
 *         . . .
 *        IMDATA_TYPE imdata[nx*ny-1]
 *   (next image)
 */

#ifndef ERROR_REPORTSIM
#define ERROR_REPORTSIM(er) if(global_error_report) (*global_error_report)(er,__FILE__,__LINE__);
#endif

#ifndef LOG_INFOSIM
#define LOG_INFOSIM(li) if(global_log_info) (*global_log_info)(li);
#endif

///Convenience structure for an image
/** Do not use this to access a shared image unless it has been retrieved via sharedim_stack::get_image(int)
  * so that the imdata pointer is set correctly for the calling process' memory map.
  */
template <class IMDATA_TYPE> struct sharedim
{
   int nx;
   int ny;
   int depth;
   long frameNo;
   int saved;
   timeval frame_time;
   timeval frame_time_dma;
   IMDATA_TYPE * imdata;  
};

typedef sharedim<short> sharedimS;
typedef sharedim<double> sharedimD;

///Convenience structure for the stack header.
struct sharedim_stack_header
{
   int max_n_images;
   int n_images;
   int last_image;
   int last_image_abs;
   int save_sequence; //A sequential number, updated when a new save sequence is desired.
   
};

///Class to manage a stack of images in shared memory
/** Can be used by both the writer and the readers
  */
template <class IMDATA_TYPE> class sharedim_stack
{
public:
   sharedim_stack();///<Constructor
   
protected:
   void * shmemptr;///<The pointer to the shared memory block
   key_t shmemkey;///<The key used to lookup the shared memory
   int   shmemid;///<The shared memory id
   size_t shmemsz;///<Size of the shared memory block
   
   intptr_t * image_list;///<Convenient pointer to the list of image offsets
   
   struct shmid_ds shmstats;///<Used to retrieve shared memory block size.
   
public:
   sharedim_stack_header * header;///<Convenient pointer to the stack header, has same value as \ref shmemptr
   
   int next_last_image;
   
public:
   void * get_shmemptr(){return shmemptr;}
   key_t get_shmemkey(){return shmemkey;}
   int   get_shmemid(){return shmemid;}
   size_t get_shmemsz(){return shmemsz;}
   
   
   int get_n_images();///<Returns the value of n_images currently in the header
   
   int set_max_n_images(int mni);///<Sets max_n_images in the header
   int get_max_n_images();///<Returns the value of max_n_images currently in the header
   
   int get_last_image();///<Returns the value of last_image currently in the header
   int get_last_image_abs(){return header->last_image_abs;}
   
   sharedim<IMDATA_TYPE> * calc_im_pos(int imno); ///<Calculates the offset of the image given by imno, takes into account max_n_images and available space.
   
   int set_saved(int imno, int sv); ///<Update the saved field of this image.
   
   int create_shm(key_t mkey, size_t sz);///<For the writer process, creates then attaches the shared memory.
   int create_shm(key_t mkey, int nims, size_t imsz);///<For the writer process, creates then attaches the shared memory.
   int attach_shm(key_t mkey);///<Attachess the shared memory.  A Reader process should start here.
   
   sharedim<IMDATA_TYPE> * set_next_image(int nx, int ny);///<Sets the next image, and returns a pointer to it, but does not increment last_image.
   int enable_next_image();///<Increments last_image
   sharedim<IMDATA_TYPE> get_image(int imno);///<Gets an image, as a sharedim structure, with the imdata pointer properly set for the calling process.
};

template <class IMDATA_TYPE> sharedim_stack<IMDATA_TYPE>::sharedim_stack()
{
   shmemptr = 0;
   shmemkey = -1;
   shmemid = 0;
   shmemsz = 0;
   
   header = 0;
   
   image_list = 0;
   
   next_last_image = -1;
}

template <class IMDATA_TYPE> int sharedim_stack<IMDATA_TYPE>::get_n_images()
{
   if(shmemptr > 0)
   {
      return header->n_images;
   }
   else return -1;
}

template <class IMDATA_TYPE> int sharedim_stack<IMDATA_TYPE>::set_max_n_images(int mni)
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

template <class IMDATA_TYPE> int sharedim_stack<IMDATA_TYPE>::get_max_n_images()
{
   if(shmemptr > 0)
   {
      return header->max_n_images;
   }
   else return -1;
}

template <class IMDATA_TYPE> int sharedim_stack<IMDATA_TYPE>::get_last_image()
{
   if(shmemptr > 0)
   {
      return header->last_image;
   }
   else return -1;
}

template <class IMDATA_TYPE> int sharedim_stack<IMDATA_TYPE>::set_saved(int imno, int sv)
{
   sharedim<IMDATA_TYPE> *simraw;
   
   if(imno < header->n_images)
   {
      simraw = (sharedim<IMDATA_TYPE> *)((intptr_t) header + (intptr_t) image_list[imno]);
      
      simraw->saved = sv;
      return 0;
   }
   
   return -1;
   
}
   

template <class IMDATA_TYPE> sharedim<IMDATA_TYPE> * sharedim_stack<IMDATA_TYPE>::calc_im_pos(int imno)
{
   sharedim<IMDATA_TYPE> * previm;
   
   if(shmemptr > 0)
   {
      if(imno > header->n_images)
      {
         ERROR_REPORTSIM("Can't calculate image offset without previous image.");
         return 0;
      }
      
      if(imno == 0) //this is easy
      {
         return (sharedim<IMDATA_TYPE> *)((intptr_t)header + image_list[0]);
      }
      
      //The previous image is set up already, so we use its image_list entry
      previm = (sharedim<IMDATA_TYPE> *)((intptr_t)header + image_list[imno-1]);

      //The correct way:
      size_t offset = (sizeof(sharedim<IMDATA_TYPE>) + (previm->nx*previm->ny*sizeof(IMDATA_TYPE)));
      previm = (sharedim<IMDATA_TYPE> *)((intptr_t) previm +   (size_t) offset);
      
      return previm;
   }
   else return 0; //can't do anything
}

template <class IMDATA_TYPE> int sharedim_stack<IMDATA_TYPE>::create_shm(key_t mkey, size_t sz)
{
   int rv;
   std::ostringstream oss;
   
   errno = 0;
   if((shmemid = shmget(mkey, sz, IPC_CREAT | 0666))<0)
   {
      perror("shmget");
      if(errno == EINVAL)
      {
         oss.str("");
         oss << "shmget returned EINVAL.  Check kernel SHMMAX paramter in /etc/sysctl.conf.";
         oss << " sz = " << sz;
         ERROR_REPORTSIM(oss.str().c_str());
         return -1;
      }
      
      shmemid = shmget(mkey, 1, 0666);
      
      //If it failed, try to remove the shmem block and recreate it.
      
      if(shmctl(shmemid, IPC_RMID, 0) < 0)
      {
         oss.str("");
         oss << "Could not remove shared memory with key " << mkey << " and ID " << shmemid << ".";
         ERROR_REPORTSIM(oss.str().c_str());
         return -1;
      }
      oss << "Removed shared memory with key " << mkey << ".";
      LOG_INFOSIM(oss.str().c_str());
      oss.str("");
      
      if((shmemid = shmget(mkey, sz, IPC_CREAT | 0666))<0)
      {
         oss << "Could not create shared memory with key " << mkey << ".";
         ERROR_REPORTSIM(oss.str().c_str());
         return -1;
      }
   }
   
   oss << "Shared memory created with key " << mkey << ".";
   LOG_INFOSIM(oss.str().c_str());
   
   rv = attach_shm(mkey);
   
   return rv;
}

template <class IMDATA_TYPE> int sharedim_stack<IMDATA_TYPE>::create_shm(key_t mkey, int nims, size_t sz)
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

template <class IMDATA_TYPE> int sharedim_stack<IMDATA_TYPE>::attach_shm(key_t mkey)
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
            ERROR_REPORTSIM(oss.str().c_str());
         }
         shmemid = 0;
         reported = 1;
         return -1;
      }
   }
   
   if ((shmemptr = shmat(shmemid, 0, 0)) == (char *) -1)
   {
      if(!reported)
      {
         oss << "Could not attach shared memory with key " << mkey;
         ERROR_REPORTSIM(oss.str().c_str());
      }
      shmemptr = 0;
      reported = 1;
      return -1;
   }
   
   if (shmctl(shmemid, IPC_STAT, &shmstats) < 0)
   {
      if(!reported)
      {
         oss << "Could not get shared memory stats with key " << mkey;
         ERROR_REPORTSIM(oss.str().c_str());
      }
      reported = 1;
      return -1;
   }
   
   shmemkey = mkey;
   shmemsz = shmstats.shm_segsz;
   
   oss << "Attached to shared memory with key " << mkey << " of size: " << shmemsz;
   LOG_INFOSIM(oss.str().c_str());
   
   header = (sharedim_stack_header *) shmemptr;
   
   image_list = (intptr_t *) ((intptr_t) shmemptr + sizeof(sharedim_stack_header));
   
   return 0;
}

template <class IMDATA_TYPE> sharedim<IMDATA_TYPE> * sharedim_stack<IMDATA_TYPE>::set_next_image(int nx, int ny)
{
   sharedim<IMDATA_TYPE> * nextim;
   sharedim<IMDATA_TYPE> * nextnextim;
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
      
      offset =  sizeof(sharedim<IMDATA_TYPE>) + ((intptr_t)nx)*((intptr_t)ny)*sizeof(IMDATA_TYPE);
      
      nextnextim = (sharedim<IMDATA_TYPE> *)((intptr_t)nextim + offset);
      
      //Check if we'll exceed the shared memory size, wrap around if so.
      if( (intptr_t)nextnextim -  (intptr_t) shmemptr  > (intptr_t)(shmemsz-1))
      {
         std::cerr << "Wrapping " << (header->last_image) + 1 << " " << (intptr_t)nextnextim -  (intptr_t) shmemptr << "\n";
         next_last_image = 0;
         nextim = (sharedim<IMDATA_TYPE>*)((intptr_t)header + image_list[0]);
      }
      else
      {
         //Nope, just use the next available image
         next_last_image = header->last_image + 1;
         image_list[next_last_image] = (intptr_t)nextim-(intptr_t)header;
      }
   }
   else //Just wrapping around
   {
      nextim = (sharedim<IMDATA_TYPE> *)((intptr_t)header + (intptr_t)image_list[0]);
      next_last_image = 0;
   }
   
   //Now fill in the structure
   nextim->nx = nx;
   nextim->ny = ny;
   nextim->frameNo = -1;
   nextim->imdata = (IMDATA_TYPE *) ((intptr_t)nextim+sizeof(sharedim<IMDATA_TYPE>));   //this pointer is only valid in the calling process
   
   return nextim;
}

template <class IMDATA_TYPE> int sharedim_stack<IMDATA_TYPE>::enable_next_image()
{
   header->last_image = next_last_image;
   if(header->n_images <= next_last_image) header->n_images++;
   header->last_image_abs++;
   return 0;
}

template <class IMDATA_TYPE> sharedim<IMDATA_TYPE> sharedim_stack<IMDATA_TYPE>::get_image(int imno)
{
   sharedim<IMDATA_TYPE> *simraw, sim;
   
   //Initialize the sharedim struct (should have an initializer)
   sim.nx =0;
   sim.ny =0;
   sim.depth = 0;
   sim.imdata = 0;
   
   if(imno < header->n_images)
   {
      simraw = (sharedim<IMDATA_TYPE> *)((intptr_t) header + (intptr_t) image_list[imno]);
      
      sim.nx = simraw->nx;
      sim.ny = simraw->ny;
      sim.depth = simraw->depth;
      sim.frameNo = simraw->frameNo;
      sim.frame_time = simraw->frame_time;
      sim.frame_time_dma = simraw->frame_time_dma;
      sim.saved = simraw->saved;
      sim.imdata = (IMDATA_TYPE *) ((intptr_t)simraw + sizeof(sharedim<IMDATA_TYPE>));
   }
   return sim;
   
}

typedef sharedim_stack<short> sharedim_stackS;
typedef sharedim_stack<double> sharedim_stackD;
typedef sharedim_stack<float> sharedim_stackF;
typedef sharedim_stack<unsigned char> sharedim_stackUC;

#endif //__SHAREDIM_STACK_H__
