
#include "imviewer.h"

//float (*pixget)(void *, size_t);

imviewer::imviewer(int data_type, imviewer_shmt shkey, QWidget * Parent, Qt::WindowFlags f) : QWidget(Parent, f)
{
   ZoomLevel_min = .125;
   ZoomLevel_max = 64.;
   //ZoomLevel_intmax = 280;
   ZoomLevel = 1.0;
   
   nx = 0;
   ny = 0;
   imdata = 0;
   tmpim = 0;
   localImdata = false;
   
   IMDATA_TYPE = data_type;
  
   this->pixget = getPixPointer(IMDATA_TYPE);//&getPix<IMDATA_TYPE>;
   IMDATA_TYPE_SIZE = sizeof_imv_type(IMDATA_TYPE);//IMDATA_TYPE);
   
   //imdata_sortdex = 0;

#if RT_SYSTEM == RT_SYSTEM_VISAO   
   dark_sim.imdata = 0;
#endif
   qim = 0;
   
   /*** Color map ***/
   mincol = 0;
   maxcol = 256;
   abs_fixed = true;
   rel_fixed = false;
   
   colorbar_mode = minmaxglobal;
   colorbar_type = typelinear;
   current_colorbar = colorbarGrey;
   //load_colorbar();
   
   /*** Real Time Setup ***/
   //Defaulting to VisAO shared image stack
   RealTimeEnabled = true;
   RealTimeStopped = false;
   RealTimeProtocol = 0;
   imtimer_timeout = 20;

   shmem_attached = 0;
   shmem_key = shkey;
   connect(&imtimer, SIGNAL(timeout()), this, SLOT(_shmem_timerout()));
   
   /*** Real Time FPS ***/
   i_fps = -1;
   n_ave_fps = 20;
   
   sat_level = 0;
   saturated = 0;
   
   //acc_dens_cachesize = 10000;
   //acc_dens_numbins = 1000;
   
   //mean_acc = new accumulator_set<float, stats<tag::mean,tag::min,tag::max > >;
   //median_acc = new accumulator_set<float, stats<tag::median(with_density) > >(density_cache_size=acc_dens_cachesize, density_num_bins=acc_dens_numbins);
   //median_acc = new accumulator_set<float, stats<tag::median(with_p_square_quantile) > >;

   userBoxActive = 0;
   
   applyDark = 0;
   applyDarkChanged= 1;
   
   last_saved = -1;
   curr_saved = 0;
}


void imviewer::setUserBoxActive(bool usba)
{
   if(usba)
   {
      int idx;
      float imval;
      
      if(userBox_i0 > userBox_i1)
      {
         idx = userBox_i0;
         userBox_i0 = userBox_i1;
         userBox_i1 = idx;
      }
      
      if(userBox_i0 < 0) userBox_i0 = 0;
      if(userBox_i0 >= nx) userBox_i0 = nx-(userBox_i1-userBox_i0);

      if(userBox_i1 <= 0) userBox_i1 = 0 + (userBox_i1-userBox_i0);
      if(userBox_i0 > nx) userBox_i1 = nx-1;

      if(userBox_j0 > userBox_j1)
      {
         idx = userBox_j0;
         userBox_j0 = userBox_j1;
         userBox_j1 = idx;
      }
      
      if(userBox_j0 < 0) userBox_j0 = 0;
      if(userBox_j0 >= nx) userBox_j0 = nx-(userBox_j1-userBox_j0);
      
      if(userBox_j1 <= 0) userBox_j1 = 0 + (userBox_j1-userBox_j0);
      if(userBox_j0 > nx) userBox_j1 = nx-1;

      
#if RT_SYSTEM == RT_SYSTEM_VISAO  
      bool dodark = 1;
      if(!applyDark || !dark_sim.imdata)         
      {
         dodark = 0;
      }
      //if(!applyDark || !dark_sim.imdata)         
      //{
         idx = userBox_i0*nx + userBox_j0;
         imval = pixget(imdata, idx);
         if(dodark) imval -= pixget(dark_sim.imdata, idx);
      
         userBox_min = imval;//imdata[userBox_i0*nx + userBox_j0];
         userBox_max = imval;//imdata[userBox_i0*nx + userBox_j0];
         for(int i = userBox_i0; i < userBox_i1; i++)
         {
            for(int j = userBox_j0; j < userBox_j1; j++)
            {
               idx = i*nx + j;
               imval = pixget(imdata, idx);
               if(dodark) imval -= pixget(dark_sim.imdata, idx);
      
               if(imval < userBox_min) userBox_min = imval;
               if(imval > userBox_max) userBox_max = imval;
            }
         }
      //}
      //else
     // {
//          std::cout << "ok\n";
//          idx = userBox_i0*nx + userBox_j0;
//          imval = pixget(imdata, idx);
//          imv_m_darkv imv - pixget(dark_sim->imdata, idx);
//          
//          userBox_min = imdata[userBox_i0*nx + userBox_j0]-dark_sim.imdata[userBox_i0*nx + userBox_j0];
//          userBox_max = imdata[userBox_i0*nx + userBox_j0]-dark_sim.imdata[userBox_i0*nx + userBox_j0];
//          for(int i = userBox_i0; i < userBox_i1; i++)
//          {
//             for(int j = userBox_j0; j < userBox_j1; j++)
//             {
//                idx = i*nx + j;
//       
//                if(imdata[idx]-dark_sim.imdata[idx] < userBox_min) userBox_min = imdata[idx]-dark_sim.imdata[idx];
//                if(imdata[idx]-dark_sim.imdata[idx] > userBox_max) userBox_max = imdata[idx]-dark_sim.imdata[idx];
//             }
//          }
//          std::cout << userBox_min << " " << userBox_max << "\n";
//       }
#else
      idx = userBox_i0*nx + userBox_j0;
      imval = pixget(imdata, idx);
         
      userBox_min = imval;
      userBox_max = imval;
      for(int i = userBox_i0; i < userBox_i1; i++)
      {
         for(int j = userBox_j0; j < userBox_j1; j++)
         {
            idx = i*nx + j;
            imval = pixget(imdata, idx);
      
            if(imval < userBox_min) userBox_min = imval;
            if(imval > userBox_max) userBox_max = imval;
         }
      }
#endif
      
      
      set_mindat(userBox_min);
      set_maxdat(userBox_max);
      //std::cout << userBox_min << " " << userBox_max << "\n";
      userBoxActive = usba;
      set_colorbar_mode(minmaxbox);
      changeImdata(false);
      return;
   }
   userBoxActive = usba;

   post_setUserBoxActive(usba);
   
}
      
void imviewer::allocImdata(int x, int y)
{
   if(imdata && localImdata) delete[] imdata;
   
   setImsize(x, y);
   
   imdata = new char[nx*ny*IMDATA_TYPE_SIZE];
   
   localImdata = true;
   changeImdata(true);
}

void imviewer::setImsize(int x, int y)
{
   int cb;
   
   if(nx !=x  || ny !=y  || qim == 0)
   {
      if(x!=0 && y!=0)
      {
         nx = x;
         ny = y;
         
         if(qim) delete qim;
         
         qim = new QImage(nx, ny, QImage::Format_Indexed8);
         
         cb = current_colorbar; //force a reload.
         current_colorbar = -1;
         
         load_colorbar(cb);
         
#if RT_SYSTEM == RT_SYSTEM_VISAO
         if(dark_sim.imdata) dark_sim = dark_sis->get_image(0);
#endif
         postSetImsize();
      }
   }
}

void imviewer::postSetImsize()
{
   return;
}

void imviewer::changeImdata(bool newdata)
{
   #if RT_SYSTEM == RT_SYSTEM_VISAO  
   if(applyDark && dark_sim.imdata)
   {
      changeImdata_applyDark(newdata);
      return;
   }
   #endif         
   
   //float pixval;
   float tmp_min;
   float tmp_max;
   //float tmp_mean;
   
   int idx;
   float imval;
   
   if(!imdata) return;
   
   amChangingimdata = true;
   
   if(!newdata)
   {
      changeImdataRecolorOnly();
   }
   else
   {
      //Update statistics
      imval = pixget(imdata,0);
      tmp_min = imval;
      tmp_max = imval;
      saturated = 0;
      
      //tmp_mean = 0;
      

      if(userBoxActive)
      {
         idx = userBox_i0*nx + userBox_j0;
         imval = pixget(imdata, idx);
         userBox_min = imval;
         userBox_max = imval;
      }   

      for(int i = 0; i < nx; i++)
      {
         for(int j=0;j < ny; j++)
         {
            idx = i*nx + j;
            imval = pixget(imdata, idx);
            //(*mean_acc)(imdata[idx]);
            //tmp_mean += imdata[idx];
            if(imval > tmp_max) tmp_max = imval;
            if(imval < tmp_min) tmp_min = imval;
            
            if(imval >= sat_level) saturated++;

            if(userBoxActive)
            {
               if(i>=userBox_i0 && i<userBox_i1 && j>=userBox_j0 && j < userBox_j1)
               {
                  if(imval < userBox_min) userBox_min = imval;
                  if(imval > userBox_max) userBox_max = imval;
               }
            }
            
            qim->setPixel(j, nx-i-1, (int)calcPixval(imval));
            
         }
      }
      
      imdat_max = tmp_max;//max(*mean_acc);
      imdat_min = tmp_min;// min(*mean_acc);
      //imdat_mean = tmp_mean/(nx*ny);//mean(*mean_acc);

      mindat_rel = (mindat - imdat_min)/(imdat_max-imdat_min);
      maxdat_rel = (maxdat - imdat_min)/(imdat_max-imdat_min);

      
    }
    qpm = QPixmap::fromImage(*qim,Qt::ThresholdDither);
       
    postChangeImdata();
    amChangingimdata = false;
}

void imviewer::changeImdataRecolorOnly()
{
   for(int i = 0; i < nx; i++)
   {
      for(int j=0;j <ny; j++)
      {
         qim->setPixel(j, nx-i-1, (int)calcPixval( pixget(imdata,i*nx + j) ));
      }
   }
}

void imviewer::changeImdata_applyDark(bool newdata)
{
    #if RT_SYSTEM == RT_SYSTEM_VISAO     
   //float pixval;
   float tmp_min;
   float tmp_max;
   //float tmp_mean;
   
   int idx;
   float imval, darkval, imv_m_darkv;
   
   if(!imdata) return;
   
   amChangingimdata = true;
   
   if(!newdata)
   {
      changeImdataRecolorOnly_applyDark();
   }
   else
   {
      //Update statistics
      imval = pixget(imdata, 0);
      darkval = pixget(dark_sim.imdata, 0);
      imv_m_darkv = imval-darkval;
      
      tmp_min = imv_m_darkv;
      tmp_max = imv_m_darkv;
      saturated = 0;
      
      //tmp_mean = 0;
      

      if(userBoxActive)
      {
         idx = userBox_i0*nx + userBox_j0;
         imval = pixget(imdata, idx);
         darkval = pixget(dark_sim.imdata, idx);
         imv_m_darkv = imval-darkval;
      
         userBox_min = imv_m_darkv;
         userBox_max = imv_m_darkv;
      }   

      for(int i = 0; i < nx; i++)
      {
         for(int j=0;j < ny; j++)
         {
            idx = i*nx + j;
            imval = pixget(imdata, idx);
            darkval = pixget(dark_sim.imdata, idx);
            imv_m_darkv = imval-darkval;
         
            //(*mean_acc)(imdata[idx]);
            //tmp_mean += imdata[idx];
            if(imv_m_darkv > tmp_max) tmp_max = imv_m_darkv;
            if(imv_m_darkv < tmp_min) tmp_min = imv_m_darkv;
            
            if(imval >= sat_level) saturated++;

            if(userBoxActive)
            {
               if(i>=userBox_i0 && i<userBox_i1 && j>=userBox_j0 && j < userBox_j1)
               {
                  if(imv_m_darkv < userBox_min) userBox_min = imv_m_darkv;
                  if(imv_m_darkv > userBox_max) userBox_max = imv_m_darkv;
               }
            }
            
            qim->setPixel(j, nx-i-1, (int)calcPixval(imv_m_darkv));
            
         }
      }
      
      imdat_max = tmp_max;//max(*mean_acc);
      imdat_min = tmp_min;// min(*mean_acc);
      //imdat_mean = tmp_mean/(nx*ny);//mean(*mean_acc);

      mindat_rel = (mindat - imdat_min)/(imdat_max-imdat_min);
      maxdat_rel = (maxdat - imdat_min)/(imdat_max-imdat_min);

      
   }
   qpm = QPixmap::fromImage(*qim,Qt::ThresholdDither);
       
   postChangeImdata();
   amChangingimdata = false;
   
#endif
}

void imviewer::changeImdataRecolorOnly_applyDark()
{
   #if RT_SYSTEM == RT_SYSTEM_VISAO  
   int idx;
   
   for(int i = 0; i < nx; i++)
   {
      for(int j=0;j <ny; j++)
      {
         idx = i*nx + j;
         qim->setPixel(j, nx-i-1, (int)calcPixval(pixget(imdata,idx)-pixget(dark_sim.imdata, idx)));
      }
   }
#endif
}

float imviewer::calcPixval(float d)
{
   float pixval;
   float a = 1000;
   static float log10_a = log10(a);
   /*if(colorbar_type == typelog)
    *       {
    *               d = log10(d);
    }*/
   
   pixval = d - mindatsc;
   if(pixval < 0) pixval = 0;

   if(pixval > maxdatsc - mindatsc) pixval = maxdatsc-mindatsc;
        
   if(maxdatsc > mindatsc) pixval = pixval/((float)(maxdatsc-mindatsc));
   else pixval = .5;

   switch(colorbar_type)
   {
      case typelog:
         pixval = log10(a*pixval+1.)/log10_a;
         break;
      case typepow:
         pixval = (pow(a, pixval) - 1.)/a;
         break;
      case typesqrt:
         pixval = sqrt(pixval);
         break;
      case typesquare:
         pixval = pixval*pixval;
         break;
      default:
         break;
   }
   
   if(pixval > 1.) pixval = 1.;
   if(pixval < 0.) pixval = 0.;
   
   return pixval*((float)(maxcol-1-mincol));
   
}

void imviewer::postChangeImdata()
{
   return;
}

void imviewer::point_imdata(void * imd)
{
   
   if(imdata && localImdata) delete[] imdata;
   imdata = (char *)imd;
   
   localImdata = false;
   changeImdata(true);
}

void imviewer::point_imdata(int x, int y, void * imd)
{
   setImsize(x,y);
   point_imdata(imd);
}

void imviewer::set_mindat(float md)
{
   mindat = md;
   if(colorbar_type == typelinear)
   {
      mindatsc = mindat;
   }
   if(colorbar_type == typelog)
   {
      mindatsc = 0;
      
   }
}

void imviewer::set_maxdat(float md)
{
   maxdat = md;
   if(colorbar_type == typelinear)
   {
      maxdatsc = maxdat;
   }
   if(colorbar_type == typelog)
   {
      //if(maxdat > 0) maxdatsc = log10(maxdat);
      //else
      maxdatsc = maxdat;
   }
}

void imviewer::set_bias(float b)
{
   float cont = get_contrast();
   
   set_mindat(b - 0.5*cont);
   set_maxdat(b + 0.5*cont);
}

void imviewer::set_bias_rel(float br)
{
   float cont = get_contrast();
   
   set_mindat(imdat_min + br*(imdat_max-imdat_min) - 0.5*cont);
   set_maxdat(imdat_min + br*(imdat_max-imdat_min) + 0.5*cont);
}

void imviewer::set_contrast(float c)
{
   float b = get_bias();
   set_mindat(b - 0.5*c);
   set_maxdat(b + 0.5*c);
}

void imviewer::set_contrast_rel(float cr)
{
   float b = get_bias();
   set_mindat(b - .5*(imdat_max-imdat_min)/cr);
   set_maxdat(b + .5*(imdat_max-imdat_min)/cr);
}

int load_colorbar_jet(QImage *qim)
{
   int i = 0;
   qim->setNumColors(64);
   qim->setColor(i++, qRgb(  0,   0, 144 ));
   qim->setColor(i++, qRgb(  0,   0, 160 ));
   qim->setColor(i++, qRgb(  0,   0, 176 ));
   qim->setColor(i++, qRgb(  0,   0, 192 ));
   qim->setColor(i++, qRgb(  0,   0, 208 ));
   qim->setColor(i++, qRgb(  0,   0, 224 ));
   qim->setColor(i++, qRgb(  0,   0, 240 ));
   qim->setColor(i++, qRgb(  0,   0, 255 ));
   qim->setColor(i++, qRgb(  0,  16  , 255 ));
   qim->setColor(i++, qRgb(  0,  32  , 255 ));
   qim->setColor(i++, qRgb(  0,  48  , 255 ));
   qim->setColor(i++, qRgb(  0,  64  , 255 ));
   qim->setColor(i++, qRgb(  0,  80, 255 ));
   qim->setColor(i++, qRgb(  0,  96  , 255 ));
   qim->setColor(i++, qRgb(  0, 112  , 255 ));
   qim->setColor(i++, qRgb(  0, 128  , 255 ));
   qim->setColor(i++, qRgb(  0, 144  , 255 ));
   qim->setColor(i++, qRgb(  0, 160, 255 ));
   qim->setColor(i++, qRgb(  0, 176  , 255 ));
   qim->setColor(i++, qRgb(  0, 192  , 255 ));
   qim->setColor(i++, qRgb( 0 , 208  , 255 ));
   qim->setColor(i++, qRgb(  0, 224  , 255 ));
   qim->setColor(i++, qRgb(  0, 240, 255 ));
   qim->setColor(i++, qRgb(  0, 255  , 255 ));
   qim->setColor(i++, qRgb( 16  , 255  , 240 ));
   qim->setColor(i++, qRgb( 32  , 255  , 224 ));
   qim->setColor(i++, qRgb( 48  , 255  , 208 ));
   qim->setColor(i++, qRgb( 64  , 255  , 192 ));
   qim->setColor(i++, qRgb( 80, 255  , 176 ));
   qim->setColor(i++, qRgb( 96  , 255  , 160 ));
   qim->setColor(i++, qRgb(112  , 255  , 144 ));
   qim->setColor(i++, qRgb(128  , 255  , 128 ));
   qim->setColor(i++, qRgb(144  , 255  , 112 ));
   qim->setColor(i++, qRgb(160, 255  ,  96 ));
   qim->setColor(i++, qRgb(176  , 255  ,  80 ));
   qim->setColor(i++, qRgb(192  , 255  ,  64 ));
   qim->setColor(i++, qRgb(208  , 255  ,  48 ));
   qim->setColor(i++, qRgb(224  , 255  ,  32 ));
   qim->setColor(i++, qRgb(240, 255  ,  16 ));
   qim->setColor(i++, qRgb(255  , 255  ,   0 ));
   qim->setColor(i++, qRgb(255  , 240,   0 ));
   qim->setColor(i++, qRgb(255  , 224  ,   0 ));
   qim->setColor(i++, qRgb(255  , 208  ,   0 ));
   qim->setColor(i++, qRgb(255  , 192  ,   0 ));
   qim->setColor(i++, qRgb(255  , 176  ,   0 ));
   qim->setColor(i++, qRgb(255  , 160,   0 ));
   qim->setColor(i++, qRgb(255  , 144  ,   0 ));
   qim->setColor(i++, qRgb(255  , 128  ,   0 ));
   qim->setColor(i++, qRgb(255  , 112  ,   0 ));
   qim->setColor(i++, qRgb(255  ,  96  ,   0 ));
   qim->setColor(i++, qRgb(255  ,  80,   0 ));
   qim->setColor(i++, qRgb(255  ,  64  ,   0 ));
   qim->setColor(i++, qRgb(255  ,  48  ,   0 ));
   qim->setColor(i++, qRgb(255  ,  32  ,   0 ));
   qim->setColor(i++, qRgb(255  ,  16  ,   0 ));
   qim->setColor(i++, qRgb(255  ,   0,   0 ));
   qim->setColor(i++, qRgb(240,   0,   0 ));
   qim->setColor(i++, qRgb(224  ,   0,   0 ));
   qim->setColor(i++, qRgb(208  ,   0,   0 ));
   qim->setColor(i++, qRgb(192  ,   0,   0 ));
   qim->setColor(i++, qRgb(176  ,   0,   0 ));
   qim->setColor(i++, qRgb(160,   0,   0 ));
   qim->setColor(i++, qRgb(144  ,   0,   0 ));
   qim->setColor(i, qRgb(128  ,   0,   0 ));
   return i;
}

int load_colorbar_hot(QImage *qim)
{
   int i = 0;

   qim->setNumColors(64);
   
   qim->setColor(i++, qRgb(   11,  0, 0));
   qim->setColor(i++, qRgb(   21,  0, 0));
   qim->setColor(i++, qRgb(   32,  0, 0));
   qim->setColor(i++, qRgb(   43,  0, 0));
   qim->setColor(i++, qRgb(   53,  0, 0));
   qim->setColor(i++, qRgb(   64,  0, 0));
   qim->setColor(i++, qRgb(   75,  0, 0));
   qim->setColor(i++, qRgb(   85,  0, 0));
   qim->setColor(i++, qRgb(   96,  0, 0));
   qim->setColor(i++, qRgb(  107,  0, 0));
   qim->setColor(i++, qRgb(  117,  0, 0));
   qim->setColor(i++, qRgb(  128,  0, 0));
   qim->setColor(i++, qRgb(  139,  0, 0));
   qim->setColor(i++, qRgb(  149,  0, 0));
   qim->setColor(i++, qRgb(  160,  0, 0));
   qim->setColor(i++, qRgb(  171,  0, 0));
   qim->setColor(i++, qRgb(  181,  0, 0));
   qim->setColor(i++, qRgb(  192,  0, 0));
   qim->setColor(i++, qRgb(  203,  0, 0)); 
   qim->setColor(i++, qRgb(  213,  0, 0));
   qim->setColor(i++, qRgb(  224,  0, 0));
   qim->setColor(i++, qRgb(  235,  0, 0));
   qim->setColor(i++, qRgb(  245,  0, 0));
   qim->setColor(i++, qRgb(  255,  0, 0));
   qim->setColor(i++, qRgb(  255,   11 , 0));
   qim->setColor(i++, qRgb(  255,   21 , 0));
   qim->setColor(i++, qRgb(  255,   32 , 0));
   qim->setColor(i++, qRgb(  255,   43 , 0));
   qim->setColor(i++, qRgb(  255,   53 , 0));
   qim->setColor(i++, qRgb(  255,   64 , 0));
   qim->setColor(i++, qRgb(  255,   75 , 0));
   qim->setColor(i++, qRgb(  255,   85 , 0));
   qim->setColor(i++, qRgb(  255,   96 , 0));
   qim->setColor(i++, qRgb(  255,  107 , 0));
   qim->setColor(i++, qRgb(  255,  117 , 0));
   qim->setColor(i++, qRgb(  255,  128 , 0));
   qim->setColor(i++, qRgb(  255,  139 , 0));
   qim->setColor(i++, qRgb(  255,  149 , 0));
   qim->setColor(i++, qRgb(  255,  160 , 0));
   qim->setColor(i++, qRgb(  255,  171 , 0));
   qim->setColor(i++, qRgb(  255,  181 , 0));
   qim->setColor(i++, qRgb(  255,  192 , 0));
   qim->setColor(i++, qRgb(  255,  202 , 0));
   qim->setColor(i++, qRgb(  255,  213 , 0));
   qim->setColor(i++, qRgb(  255,  224 , 0));
   qim->setColor(i++, qRgb(  255,  235 , 0));
   qim->setColor(i++, qRgb(  255,  245 , 0));
   qim->setColor(i++, qRgb(  255,  255 , 0));
   qim->setColor(i++, qRgb(  255,  255 ,  16));
   qim->setColor(i++, qRgb(  255,  255 ,  32));
   qim->setColor(i++, qRgb(  255,  255 ,  48));
   qim->setColor(i++, qRgb(  255,  255 ,  64));
   qim->setColor(i++, qRgb(  255,  255 ,  80));
   qim->setColor(i++, qRgb(  255,  255 ,  96));
   qim->setColor(i++, qRgb(  255,  255 , 112));
   qim->setColor(i++, qRgb(  255,  255 , 128));
   qim->setColor(i++, qRgb(  255,  255 , 144));
   qim->setColor(i++, qRgb(  255,  255 , 160));
   qim->setColor(i++, qRgb(  255,  255 , 176));
   qim->setColor(i++, qRgb(  255,  255 , 192));
   qim->setColor(i++, qRgb(  255,  255 , 208));
   qim->setColor(i++, qRgb(  255,  255 , 224));
   qim->setColor(i++, qRgb(  255,  255 , 240));
   qim->setColor(i,   qRgb(  255,  255 , 255));

   return i;
}

int load_colorbar_bone(QImage *qim)
{
   int i = 0;

   qim->setNumColors(64);
    
   qim->setColor(i++, qRgb(     0,  0  ,      1  ));
   qim->setColor(i++, qRgb(     4  ,         4     ,      6  ));
   qim->setColor(i++, qRgb(     7  ,         7     ,     11  ));
   qim->setColor(i++, qRgb(    11  ,        11     ,     16  ));
   qim->setColor(i++, qRgb(    14  ,        14     ,     21  ));
   qim->setColor(i++, qRgb(    18  ,        18     ,     26  ));
   qim->setColor(i++, qRgb(    21  ,        21     ,     31  ));
   qim->setColor(i++, qRgb(    25  ,        25     ,     35  ));
   qim->setColor(i++, qRgb(    28  ,        28     ,     40));
   qim->setColor(i++, qRgb(    32  ,        32     ,     45  ));
   qim->setColor(i++, qRgb(    35  ,        35     ,     50));
   qim->setColor(i++, qRgb(    39  ,        39     ,     55  ));
   qim->setColor(i++, qRgb(    43  ,        43     ,     60));
   qim->setColor(i++, qRgb(    46  ,        46     ,     65  ));
   qim->setColor(i++, qRgb(    50,        50   ,     70));
   qim->setColor(i++, qRgb(    53  ,        53     ,     74  ));
   qim->setColor(i++, qRgb(    57  ,        57     ,     79  ));
   qim->setColor(i++, qRgb(    60,        60   ,     84  ));
   qim->setColor(i++, qRgb(    64  ,        64     ,     89  ));
   qim->setColor(i++, qRgb(    67  ,        67     ,     94  ));
   qim->setColor(i++, qRgb(    71  ,        71     ,     99  ));
   qim->setColor(i++, qRgb(    74  ,        74     ,    104  ));
   qim->setColor(i++, qRgb(    78  ,        78     ,    108  ));
   qim->setColor(i++, qRgb(    81  ,        81     ,    113  ));
   qim->setColor(i++, qRgb(    85  ,        86     ,    117  ));
   qim->setColor(i++, qRgb(    89  ,        91     ,    120));
   qim->setColor(i++, qRgb(    92  ,        96     ,    124  ));
   qim->setColor(i++, qRgb(    96  ,       101     ,    128  ));
   qim->setColor(i++, qRgb(    99  ,       106     ,    131  ));
   qim->setColor(i++, qRgb(   103  ,       111     ,    135  ));
   qim->setColor(i++, qRgb(   106  ,       116     ,    138  ));
   qim->setColor(i++, qRgb(   110,       120   ,    142  ));
   qim->setColor(i++, qRgb(   113  ,       125     ,    145  ));
   qim->setColor(i++, qRgb(   117  ,       130   ,    149  ));
   qim->setColor(i++, qRgb(   120,       135     ,    152  ));
   qim->setColor(i++, qRgb(   124  ,       140   ,    156  ));
   qim->setColor(i++, qRgb(   128  ,       145     ,    159  ));
   qim->setColor(i++, qRgb(   131  ,       150   ,    163  ));
   qim->setColor(i++, qRgb(   135  ,       155     ,    166  ));
   qim->setColor(i++, qRgb(   138  ,       159     ,    170));
   qim->setColor(i++, qRgb(   142  ,       164     ,    174  ));
   qim->setColor(i++, qRgb(   145  ,       169     ,    177  ));
   qim->setColor(i++, qRgb(   149  ,       174     ,    181  ));
   qim->setColor(i++, qRgb(   152  ,       179     ,    184  ));
   qim->setColor(i++, qRgb(   156  ,       184     ,    188  ));
   qim->setColor(i++, qRgb(   159  ,       189     ,    191  ));
   qim->setColor(i++, qRgb(   163  ,       193     ,    195  ));
   qim->setColor(i++, qRgb(   166  ,       198     ,    198  ));
   qim->setColor(i++, qRgb(   172  ,       202     ,    202  ));
   qim->setColor(i++, qRgb(   178  ,       205     ,    205  ));
   qim->setColor(i++, qRgb(   183  ,       209     ,    209  ));
   qim->setColor(i++, qRgb(   189  ,       213     ,    213  ));
   qim->setColor(i++, qRgb(   194  ,       216     ,    216  ));
   qim->setColor(i++, qRgb(   200,       220   ,    220));
   qim->setColor(i++, qRgb(   205  ,       223     ,    223  ));
   qim->setColor(i++, qRgb(   211  ,       227     ,    227  ));
   qim->setColor(i++, qRgb(   216  ,       230   ,    230));
   qim->setColor(i++, qRgb(   222  ,       234     ,    234  ));
   qim->setColor(i++, qRgb(   227  ,       237     ,    237  ));
   qim->setColor(i++, qRgb(   233  ,       241     ,    241  ));
   qim->setColor(i++, qRgb(   238  ,       244     ,    244  ));
   qim->setColor(i++, qRgb(   244  ,       248     ,    248  ));
   qim->setColor(i++, qRgb(   249  ,       251     ,    251  ));
   qim->setColor(i,   qRgb(     255,         255   ,      255));
   return i;
}
      
      
      
void imviewer::load_colorbar(int cb)
{
   if(current_colorbar != cb && qim)
   {
      current_colorbar = cb;
      switch(cb)
      {
         case colorbarRed:
            mincol = 0;
            maxcol = 255;
            qim->setNumColors(256);
            for(int i=mincol; i<maxcol; i++) qim->setColor(i, qRgb(i,0,0));
            warning_color = QColor("lime");
            break;
         case colorbarGreen:
            mincol = 0;
            maxcol = 255;
            qim->setNumColors(256);
            for(int i=mincol; i<maxcol; i++) qim->setColor(i, qRgb(0,i,0));
            warning_color = QColor("magenta");
            break;
         case colorbarBlue:
            mincol = 0;
            maxcol = 255;
            qim->setNumColors(256);
            for(int i=mincol; i<maxcol; i++) qim->setColor(i, qRgb(0,0,i));
            warning_color = QColor("yellow");
            break;
         case colorbarJet:
            
            mincol = 0;
            maxcol = load_colorbar_jet(qim);
            warning_color = QColor("white");
            break;
         case colorbarHot:
            mincol = 0;
            maxcol = load_colorbar_hot(qim);
            warning_color = QColor("cyan");
            break;
         case colorbarBone:
            mincol = 0;
            maxcol = load_colorbar_bone(qim);
            warning_color = QColor("lime");
            break;
         default:
            mincol = 0;
            maxcol = 255;
            qim->setNumColors(256);
            for(int i=mincol; i<maxcol; i++) qim->setColor(i, qRgb(i,i,i));
            warning_color = QColor("lime");
            break;
      }
      changeImdata();
   }
}

void imviewer::set_colorbar_type(int ct)
{
   if(ct < 0 || ct >= colorbar_types_max)
   {
      ct = typelinear;
   }
   
   colorbar_type = ct;
   
   //now update mindatsc and maxdatasc
   set_mindat(mindat);
   set_maxdat(maxdat);
   /*      if(colorbar_type == typelinear)
    *       {
    *               mindatsc = mindat;
    *               maxdatsc = maxdat;
    }
    
    if(colorbar_type == typelog)
    {
       mindatsc = log10(mindat);
       maxdatsc = log10(maxdat);
    }*/
}

void imviewer::set_ZoomLevel(float zl)
{
   if(zl < ZoomLevel_min) zl = ZoomLevel_min;
   if(zl > ZoomLevel_max) zl = ZoomLevel_max;
   
   ZoomLevel = zl;
   
   post_set_ZoomLevel();
}

void imviewer::post_set_ZoomLevel()
{
   return;
}

void imviewer::set_RealTimeEnabled(int rte)
{
   RealTimeEnabled = (rte != 0);
}

void imviewer::set_RealTimeStopped(int rts)
{
   RealTimeStopped = (rts != 0);
   
   if(RealTimeStopped)
   {
      imtimer.stop();
      if(imdata)
      {
         if(tmpim) free(tmpim);
         tmpim = (char *) malloc(nx*ny*IMDATA_TYPE_SIZE); //new IMDATA_TYPE[nx*ny];
         //for(int i=0; i< nx*ny; i++) tmpim[i] = imdata[i];
         memcpy(tmpim, imdata, nx*ny*IMDATA_TYPE_SIZE);
         imdata = tmpim;
      }
   }
   else
   {
      imtimer.start(imtimer_timeout);
      
      if(shmem_attached) timerout();
      else shmem_timerout();
      
      if(tmpim) free(tmpim);//delete[] tmpim;
      tmpim = 0;
   }
}

void imviewer::set_RealTimeProtocol(int prot)
{
}

void imviewer::set_imtimer_timeout(int to)
{
   imtimer_timeout = to;
   imtimer.start(imtimer_timeout);
}

void imviewer::_shmem_timerout()
{
   shmem_timerout();

   if(shmem_attached)
   {
      disconnect(&imtimer, SIGNAL(timeout()), this, SLOT(_shmem_timerout()));
      connect(&imtimer, SIGNAL(timeout()), this, SLOT(_timerout()));
   }
}

#if RT_SYSTEM == RT_SYSTEM_VISAO

void imviewer::shmem_timerout()
{
   sis = new sharedim_stackS;
   if(sis->attach_shm(shmem_key) == 0)
   {
      dark_sis = new sharedim_stackS;
      if(dark_sis->attach_shm(5002) == 0)
      {
         dark_sim = dark_sis->get_image(0);//dark_sis->get_last_image());
      }
      else 
      {
         std::cout << "nulling imdata\n";
         
         dark_sim.imdata=0;
      }
      shmem_attached = 1;
   }
   else
   {
      shmem_attached = 0;
      delete sis;
   }
}
#endif

#if RT_SYSTEM == RT_SYSTEM_SCEXAO
void imviewer::shmem_timerout()
{

   int SM_fd;
   struct stat file_stat;
   IMAGE_METADATA *map;
   
   SM_fd = open(shmem_key.c_str(), O_RDWR);

   char *mapv;
   
   if(SM_fd==-1)
   {
      std::cout << "could not open " << shmem_key << "\n";
      shmem_attached = 0;
      return;
   }
   else
   {
      fstat(SM_fd, &file_stat);
   
      map = (IMAGE_METADATA*) mmap(0, file_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, SM_fd, 0);

      if (map == MAP_FAILED) 
      {
        ::close(SM_fd);
        perror("Error mmapping the file");
        shmem_attached = 0;
        return;
      }
      
      std::cout << "Attached to " << shmem_key << "\n";
      shmem_attached = 1;
      
      scexao_image.memsize = file_stat.st_size;
      scexao_image.shmfd = SM_fd;
      scexao_image.md = map;
      scexao_image.md->shared = 1;
      
      mapv = (char*) map;
      mapv += sizeof(IMAGE_METADATA);
      
      switch(scexao_image.md[0].atype)
      {
         case SCEXAO_CHAR:
            scexao_image.array.C = (char*) mapv;
            IMDATA_TYPE = IMV_CHAR;
            break;
         case SCEXAO_INT:
            scexao_image.array.I = (int*) mapv;
            IMDATA_TYPE = IMV_INT;
            break;
         case SCEXAO_FLOAT:
            scexao_image.array.F = (float*) mapv;
            IMDATA_TYPE = IMV_FLOAT;
            break;   
         case SCEXAO_DOUBLE:
            scexao_image.array.D = (double*) mapv;
            IMDATA_TYPE = IMV_DOUBLE;
            break;
         case SCEXAO_COMPLEX_FLOAT:
            scexao_image.array.CF = (complex_float*) mapv;
            IMDATA_TYPE = IMV_CMPLXFLOAT;
            break;   
         case SCEXAO_COMPLEX_DOUBLE:
            scexao_image.array.CD = (complex_double*) mapv;
            IMDATA_TYPE = IMV_CMPLXDOUBLE;
            break;
         case SCEXAO_USHORT:
            scexao_image.array.U = (unsigned short*) mapv;
            IMDATA_TYPE = IMV_USHORT;
            break;
         default:
            std::cerr << "Unknown SCExAO data type\n";
            exit(0);
      }
 
      this->pixget = getPixPointer(IMDATA_TYPE);
      IMDATA_TYPE_SIZE = sizeof_imv_type(IMDATA_TYPE);
      std::cout << "IMDATA_TYPE: " << IMDATA_TYPE << "\n";
      
   }
}
#endif

void imviewer::_timerout()
{
   timerout();
}

#if RT_SYSTEM == RT_SYSTEM_VISAO
void imviewer::timerout()
{
   int lastimage;
   static int last_lastimage = -1;
   static int last_depth = 0;
   sharedimS sim;
   
   lastimage = sis->get_last_image();
   
   if(lastimage > -1) 
   {
      sim = sis->get_image(lastimage);
      curr_saved = sim.saved;
      
   }   
   if(lastimage != last_lastimage && lastimage > -1)
   {
      
      update_fps(false);
      
      if(sim.depth != last_depth)
      {
         sat_level = pow(2., sim.depth) - 1;
      }
      last_depth = sim.depth;
      
      point_imdata(sim.nx, sim.ny, sim.imdata);
      frame_time = tv_to_curr_time(&sim.frame_time);
   }
   else update_fps(true);
   
   last_lastimage = lastimage;
   
}
#endif

#if RT_SYSTEM == RT_SYSTEM_SCEXAO
void imviewer::timerout()
{
   static int last_image = -1;
   static int last_depth = 0;
   int curr_image;
   size_t snx, sny;
   if(scexao_image.md[0].cnt0 > last_image)
   {
      if(scexao_image.md[0].size[2] > 0)
      {
         curr_image = scexao_image.md[0].cnt1 - 1;
         if(curr_image < 0) curr_image = scexao_image.md[0].size[2] - 1;
      }
      else curr_image = 0;
      
            
      last_image = scexao_image.md[0].cnt0;
      update_fps(false);
      
      snx = scexao_image.md[0].size[0];
      sny = scexao_image.md[0].size[1];
      point_imdata(snx,sny, (void *) (scexao_image.array.C + curr_image*snx*sny*IMDATA_TYPE_SIZE));
      //frame_time = tv_to_curr_time(&sim.frame_time);
   }
 
}
#endif


void imviewer::update_fps(bool NoAdvance)
{
   struct timeval tvtmp;
   double dftime, timetmp;
   static unsigned n_noadvance = 0;
   
   if(i_fps < 0)
   {
      fps_hist.clear();
      i_fps = 0;
      fps_time0 = 0.;
      fps_ave = 0.0;
   }
   gettimeofday(&tvtmp, 0);
   timetmp = (double) tvtmp.tv_sec + ((double)tvtmp.tv_usec)/1e6;
   
   if(fps_time0 > 0)
   {
      dftime = timetmp - fps_time0;
   
      //insert time
      if(fps_hist.size() < n_ave_fps)
      {
         fps_hist.push_back(dftime);
         i_fps = fps_hist.size()-1;
      }
      else
      {
         fps_hist[i_fps] = dftime;
         i_fps++;
         if((unsigned) i_fps >= n_ave_fps) i_fps = 0;
      }
      
      //calc fps_ave
      fps_sum = 0.;
      //int i0 = i_fps + 1;
      //if(i0 > fps_hist.size()) i0 = 0;
      for(unsigned i=0; i<fps_hist.size(); i++) fps_sum += fps_hist[i];
      
      fps_ave = 1./(fps_sum/fps_hist.size());
      
      if(NoAdvance)
      {
         if(n_noadvance > 1000./imtimer_timeout)
         {
            float fps_stalled = 1./((fps_sum-fps_hist[i_fps-1])/(fps_hist.size()-1));
            
            if(fps_ave < .8*fps_stalled) stale_fps();
         }
         i_fps--;
         if(fps_hist.size() < n_ave_fps) fps_hist.pop_back();
           if(i_fps < 0) i_fps = n_ave_fps-1;
           n_noadvance++;
      }
      else
      {
         n_noadvance = 0;
      }
   }
   if(!NoAdvance) fps_time0 =  timetmp;
   
   update_age();
}

void imviewer::stale_fps()
{
   return;
}

void imviewer::update_age()
{
   return;
}
