
#ifndef __imviewer_h__
#define __imviewer_h__

#include <cmath>
#include <iostream>
#include <vector>
#include <sys/time.h>

#include <QImage>
#include <QPixmap>
#include <QTimer>
#include <QWidget>

#include "pixaccess.h"


#define RT_SYSTEM_VISAO 0
#define RT_SYSTEM_SCEXAO 1

#ifndef RT_SYSTEM
   #define RT_SYSTEM 0
#endif


#if RT_SYSTEM == RT_SYSTEM_VISAO
  
   #include "sharedim_stack.h"
  
   //#define IMDATA_TYPE short
  
   typedef key_t imviewer_shmt;
   
#endif

#if RT_SYSTEM == RT_SYSTEM_SCEXAO
   //#define IMDATA_TYPE  double
   #include <cstdio>
   
   #include <unistd.h>
   #include <sys/stat.h>
   #include <sys/mman.h>
   #include <sys/file.h>
   
   #include "CLIimagestruct.h"
   
   typedef std::string imviewer_shmt;
#endif


class imviewer : public QWidget
{
   Q_OBJECT
   
   public:

      imviewer(int data_type, imviewer_shmt shkey, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   
   /*** Image Data ***/
   protected:
      int nx;
      int ny;
      float frame_time; ///<The timestamp of the current frame
      char * imdata; ///< Pointer the image data
      char * tmpim; ///<  A temporary image storage
      bool localImdata; ///< flag to determine if imdata is locally allocated
     
      float (*pixget)(void *, size_t);
      int IMDATA_TYPE;
      size_t IMDATA_TYPE_SIZE;
      
      QImage * qim; ///<A QT image, used to store the color-map encoded data
      QPixmap qpm; ///<A QT pixmap, used to prep the QImage for display.
      
      //accumulator_set<double, stats<tag::mean, tag::min, tag::max> > * mean_acc;
      //accumulator_set<double, stats<tag::median(with_density) > > * median_acc;
      //accumulator_set<double, stats<tag::median(with_p_square_quantile) > > * median_acc;
      
      //int acc_dens_cachesize;
      //int acc_dens_numbins;

      /** @name A User Define Region
       */
      //@{
      int userBoxActive;
      
      int userBox_i0;
      int userBox_i1;
      int userBox_j0;
      int userBox_j1;
      
      int guideBox_i0;
      int guideBox_i1;
      int guideBox_j0;
      int guideBox_j1;
      
      float userBox_max;
      float userBox_min;

      imviewer_shmt shmem_key;
      
   public:
      int getUserBoxActive(){ return userBoxActive; }
      void setUserBoxActive(bool usba);
      virtual void post_setUserBoxActive(bool usba) { return; }
      //@}
      
   public:
      ///Get the number of x pixels
      float getNx(){return nx;}

      ///Get the number of y pixels
      float getNy(){return ny;}

      ///Get the QPixMap pointer
      QPixmap * getPixmap(){return &qpm;}
      
      void allocImdata(int x, int y);
      void setImsize(int x, int y); ///Changes the image size, but only if necessary.
      virtual void postSetImsize(); ///<to call after set_imsize to handle allocations for derived classes
      
      ///Updates the QImage and the statistics after a new image.
      /** \param newdata determines whether statistics are calculated (true) or not (false).
       */
      void changeImdata(bool newdata = false);
      
      void changeImdataRecolorOnly();///<Only update the QImage with a new color mapping, does not recalc any statistics.
      
      void changeImdata_applyDark(bool newdata = false);
      
      void changeImdataRecolorOnly_applyDark();///<Only update the QImage with a new color mapping, does not recalc any statistics.
      
      float calcPixval(float d);///<Actually calculates the color mapped value of each pixel from 0 to 255.
      
      bool applyDark;
      bool applyDarkChanged;
      
   protected:
      bool amChangingimdata;
   public:
      
      virtual void postChangeImdata(); ///<to call after change imdata does its work.
      
      void point_imdata(void * imd); ///<Points imdata at a new array, no copying is done.
      void point_imdata(int x, int y, void * imd); ///<Points imdata at a new array, changing imsize if necessary, no copying is done.
      
      char * get_imdata(){return imdata;} ///<Returns the imdata pointer.
      
   protected:
      int sat_level;
      int saturated;
   
   /*** Color Map ***/
   protected:
      float mindat;
      float mindatsc;
      float mindat_rel;
      float maxdat;
      float maxdatsc;
      float maxdat_rel;
      
   public:
      void set_mindat(float md);
      float get_mindat(){return mindat;}
      void set_maxdat(float md);
      float get_maxdat(){return maxdat;}
      
      void set_bias(float b);
      void set_bias_rel(float b);
      float get_bias(){return 0.5*(maxdat+mindat);}
      float get_bias_rel(){return 0.5*(maxdat+mindat)/(maxdat-mindat);}
      
      void set_contrast(float c);
      void set_contrast_rel(float cr);
      float get_contrast(){return maxdat-mindat;}
      float get_contrast_rel(){return (imdat_max-imdat_min)/(maxdat-mindat);}
      
   protected:
      int mincol;
      int maxcol;
      
      bool abs_fixed;
      bool rel_fixed;
      
      int colorbar_mode;
      int colorbar_type;
      
      int current_colorbar;
      
      QColor warning_color;
      
   public:
      bool get_abs_fixed(){return abs_fixed;}
      bool get_rel_fixed(){return rel_fixed;}
      void set_abs_fixed()
      {
         abs_fixed = true;
         rel_fixed = false;
      }
      void set_rel_fixed()
      {
         rel_fixed = true;
         abs_fixed = false;
      }
      
      enum colorbars{colorbarGrey, colorbarJet, colorbarHot, colorbarBone, colorbarRed, colorbarGreen, colorbarBlue, colorbarMax};
      void load_colorbar(int);
      int get_current_colorbar(){return current_colorbar;}
      
      enum colorbar_modes{minmaxglobal, minmaxview, minmaxbox, user, colorbar_modes_max};
      void set_colorbar_mode(int mode){ colorbar_mode = mode;}
      int get_colorbar_mode(){return colorbar_mode;}
      
      enum colorbar_types{typelinear, typelog, typepow, typesqrt, typesquare, colorbar_types_max};
      void set_colorbar_type(int);
      int get_colorbar_type(){return colorbar_type;}
   
   /* Image Stats */
   protected:
      float imdat_min;
      float imdat_max;
      
   public:
      float get_imdat_min(){return imdat_min;}
      float get_imdat_max(){return imdat_max;}
      
   /*** Abstract Zoom ***/
   protected:
      float ZoomLevel;
      float ZoomLevel_min;
      float ZoomLevel_max;
      
   public:
      float get_ZoomLevel(){return ZoomLevel;}
      float get_ZoomLevel_min(){return ZoomLevel_min;}
      float get_ZoomLevel_max(){return ZoomLevel_max;}
      
      //void set_ZoomLevel(int zlint);
      void set_ZoomLevel(float zl);
      virtual void post_set_ZoomLevel();
      
   /***** Real Time *****/
   protected:
      bool RealTimeEnabled; ///<Controls whether imviewer is using real-time data.
      bool RealTimeStopped; ///<Set when user temporarily freezes real-time data viewing.
      int RealTimeProtocol; ///<Controls which protocol is used for gathering real time data.
      
      #if RT_SYSTEM == RT_SYSTEM_VISAO
      sharedim_stackS * sis; ///< Manages a VisAO shared memory image stack.
      sharedim_stackS * dark_sis; ///<The dark image shared memory
      sharedim<short>  dark_sim;
      #endif
      
      #if RT_SYSTEM == RT_SYSTEM_SCEXAO
      IMAGE scexao_image;
      #endif
      
      QTimer imtimer; ///< When this times out imviewer checks for a new image.
      int imtimer_timeout; ///<The timeout for checking for a new images.
      int shmem_attached;
      
      int curr_saved;
      int last_saved;
      
   /*** Real Time Controls ***/
   public:
      void set_RealTimeEnabled(int);
      void set_RealTimeStopped(int);
      void set_RealTimeProtocol(int);
      void set_imtimer_timeout(int);
      
   protected slots:
      void _shmem_timerout();
      void _timerout();

   protected:
      virtual void shmem_timerout();
      virtual void timerout();
      
   /*** Real time frames per second ***/
   protected:
      int i_fps;
      unsigned n_ave_fps;
      float fps_sum, fps_ave;
      double fps_time0;
      std::vector<float> fps_hist;
      void update_fps(bool NoAdvance = false);
      virtual void stale_fps();///<Called when fps is dropping due to know images.
      virtual void update_age();///<Called every timeout no matter what, to update the image age.
      
};

#endif //__imviewer_h__



