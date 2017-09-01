/************************************************************
 *    visaoimutils.cpp
 *
 * Author: Jared R. Males (jrmales@email.arizona.edu)
 *
 * Definitions for various image utility functions.
 *
 * Developed as part of the Magellan Adaptive Optics system.
 ************************************************************/

/** \file visaoimutils.cpp
 * \author Jared R. Males
 * \brief Definitions for various image utility functions.  
 * 
 */

#include "visaoimutils.h"


#ifndef ERROR_REPORT
#define ERROR_REPORT(er,f,l) if(global_error_report) (*global_error_report)(er,f,l);
#endif

void init_visao_imheader(visao_imheader * imhead)
{
   imhead->fsb = 0;
   imhead->csb = 0;
   imhead->ssb = 0;
   imhead->fw2sb = 0;
   imhead->fw3sb = 0;
   imhead->wsb = 0;
   imhead->aosb = 0;
   imhead->gsb = 0;
   imhead->vssb = 0;
   imhead->rsb = 0;
   imhead->stsb = 0;
   imhead->zsb = 0;
   imhead->hwpsb = 0;
   
}

int get_visao_filename(char * buffer, struct timeval *tv)
{
   struct tm uttime;
   time_t t0;
   
   t0 = tv->tv_sec;
   
   if(gmtime_r(&t0, &uttime) == 0)
   {
      ERROR_REPORT("Error getting UT time (gmtime_r returned 0).", __FILE__, __LINE__);
      return -1;
   }
   
   sprintf(buffer, "%04i%02i%02i%02i%02i%02i%06li",uttime.tm_year+1900, uttime.tm_mon+1, uttime.tm_mday, uttime.tm_hour, uttime.tm_min, uttime.tm_sec, tv->tv_usec);
   
   return 0;
}

int get_visao_filename(char * buffer, struct timespec *ts)
{
   struct tm uttime;
   struct timeval tv;
   
   time_t t0;

   tv.tv_sec = ts->tv_sec;
   tv.tv_usec= (long) (ts->tv_nsec/1e3 + .5);
   
   t0 = tv.tv_sec;

   if(gmtime_r(&t0, &uttime) == 0)
   {
      ERROR_REPORT("Error getting UT time (gmtime_r returned 0).", __FILE__, __LINE__);
      return -1;
   }
   
   sprintf(buffer, "%04i%02i%02i%02i%02i%02i%06li",uttime.tm_year+1900, uttime.tm_mon+1, uttime.tm_mday, uttime.tm_hour, uttime.tm_min, uttime.tm_sec, tv.tv_usec);
   
   return 0;
}

int write_visao_fits_aosys_header(fitsfile * outfptr, visao_imheader * head)
{
   //std::cout << "In Write head\n";
   int status = 0;
   if(head->aosb)
   {
      //std::cout << "Writing head\n";
      //std::cout << "->" << head->aosb->catobj << "<-\n";
      fits_update_key(outfptr, TSTRING, "OBSINST",(void*)(head->aosb->obsinst), "Observer Institution", &status);
      fits_update_key(outfptr, TSTRING, "OBSNAME",(void*)(head->aosb->obsname), "Observer Name", &status);
      fits_update_key(outfptr, TSTRING, "OBJECT",(void*)(head->aosb->catobj), "Object Name", &status);
      fits_update_key(outfptr, TINT, "UT",(void*)(&head->aosb->ut), "UT from TCS (sec since midnight)", &status);
      fits_update_key(outfptr, TINT, "ST",(void*)(&head->aosb->st), "ST from TCS (sec since midnight)", &status);
      fits_update_key(outfptr, TDOUBLE, "EPOCH",(void*)&head->aosb->epoch, "Epoch of target coordinates", &status);
      fits_update_key(outfptr, TDOUBLE, "RA",(void*)&head->aosb->ra, "Telescope pointing RA (degrees)", &status);
      fits_update_key(outfptr, TDOUBLE, "DEC",(void*)&head->aosb->dec, "Telescope pointing DEC (degrees)", &status);
      fits_update_key(outfptr, TDOUBLE, "CATRA",(void*)&head->aosb->catra, "Catalog input RA (degrees)", &status);
      fits_update_key(outfptr, TDOUBLE, "CATDEC",(void*)&head->aosb->catdec, "Catalog input DEC (degrees)", &status);
      fits_update_key(outfptr, TDOUBLE, "AZIMUTH",(void*)&head->aosb->az, "Telescope azimuth (degrees)", &status);
      fits_update_key(outfptr, TDOUBLE, "ELEVATIO",(void*)&head->aosb->el, "Telescope elevation (degrees)", &status);
      fits_update_key(outfptr, TDOUBLE, "PARANG",(void*)&head->aosb->pa, "Par. ang. at end of obs. (degrees)", &status);
      fits_update_key(outfptr, TDOUBLE, "AM",(void*)&head->aosb->am, "Airmass at end of observation", &status);
      fits_update_key(outfptr, TDOUBLE, "HA",(void*)&head->aosb->ha, "Hour angle at end of observation", &status);
      fits_update_key(outfptr, TDOUBLE, "ZD",(void*)&head->aosb->zd, "Zenith distance at end of observation", &status);
      fits_update_key(outfptr, TDOUBLE, "ROTANG",(void*)&head->aosb->rotang, "Rotator angle at end of observation (degrees)", &status);
      fits_update_key(outfptr, TDOUBLE, "ROTOFF",(void*)&head->aosb->rotoffset, "Rotator offset at end of observation (degrees)", &status);
      fits_update_key(outfptr, TSTRING, "ROTMODE",(void*)head->aosb->catrm, "Rotator mode, from catalog input", &status);

      
      //Weather data from TCS
      fits_update_key(outfptr, TDOUBLE, "WXTEMP",(void*)&head->aosb->wxtemp, "Outside temperature (C)", &status);
      fits_update_key(outfptr, TDOUBLE, "WXPRES",(void*)&head->aosb->wxpres, "Outside pressure (millibars)", &status);
      fits_update_key(outfptr, TDOUBLE, "WXHUMID",(void*)&head->aosb->wxhumid, "Outside humidity (%)", &status);
      fits_update_key(outfptr, TDOUBLE, "WXWIND",(void*)&head->aosb->wxwind, "Outside wind intensity (mph)", &status);
      fits_update_key(outfptr, TDOUBLE, "WXWDIR",(void*)&head->aosb->wxwdir, "Outside wind direction (degrees)", &status);
      fits_update_key(outfptr, TDOUBLE, "WXDEWPNT",(void*)&head->aosb->wxdewpoint, "Dew Point (C)", &status);
      fits_update_key(outfptr, TDOUBLE, "WXPWVEST",(void*)&head->aosb->wxpwvest, "Rough estimate of PWV (mm)", &status);
      fits_update_key(outfptr, TDOUBLE, "TTRUSS",(void*)&head->aosb->ttruss, "Telescope truss temperature (C)", &status);
      fits_update_key(outfptr, TDOUBLE, "TCELL",(void*)&head->aosb->tcell, "Primary mirror cell temperature (C)", &status);
      fits_update_key(outfptr, TDOUBLE, "TAMBIENT",(void*)&head->aosb->tambient, "Dome air temperature (C)", &status);
      fits_update_key(outfptr, TDOUBLE, "DIMMFWHM",(void*)&head->aosb->dimmfwhm, "DIMM seeing FWHM at zenith (arcsec)", &status);
      fits_update_key(outfptr, TINT, "DIMMTIME",(void*)&head->aosb->dimmtime, "DIMM seeing timestamp (sec since midnight)", &status);
      fits_update_key(outfptr, TDOUBLE, "MAG1FWHM",(void*)&head->aosb->mag1fwhm, "Baade (Mag1) seeing FWHM at zenith (arcsec)", &status);
      fits_update_key(outfptr, TINT, "MAG1TIME",(void*)&head->aosb->mag1time, "Baade (Mag1) seeing timestamp (sec since midnight)", &status);
      fits_update_key(outfptr, TDOUBLE, "MAG2FWHM",(void*)&head->aosb->mag2fwhm, "Clay (Mag2) seeing FWHM at zenith (arcsec)", &status);
      fits_update_key(outfptr, TINT, "MAG2TIME",(void*)&head->aosb->mag2time, "Clay (Mag2) seeing timestamp (sec since midnight)", &status);
      
      if(head->aosb->loop_on == 0) fits_update_key(outfptr, TSTRING, "AOLOOPST", (void *)"OPEN","Loop Status",  &status);
      else fits_update_key(outfptr, TSTRING, "AOLOOPST", (void *)"CLOSED","Loop status",  &status);
      
      
      fits_update_key(outfptr, TSTRING, "AOLOOPST", (void *)"NOT PROCESSED","Loop Status - added in post processing",  &status);
      
      fits_update_key(outfptr, TSTRING, "AOREC", (void*)head->aosb->reconstructor,"reconstructor",  &status);
      fits_update_key(outfptr, TINT, "AOMODES", (void*)&head->aosb->correctedmodes,"Number of corrected modes",  &status);
      fits_update_key(outfptr, TDOUBLE, "AOSPEED", (void*)&head->aosb->ccd39_freq,"AO loop speed (Hz), from CCD39",  &status);
      fits_update_key(outfptr, TINT, "AOBIN", (void*)&head->aosb->ccd39_bin,"AO WFS binning, from CCD39",  &status);
      fits_update_key(outfptr, TINT, "AOCOUNTS", (void*)&head->aosb->wfs_counts,"AO WFS counts per subap",  &status);
      fits_update_key(outfptr, TDOUBLE, "AOGAINTT", (void*)&head->aosb->loop_gain_tt,"AO loop T/T gain",  &status);
      fits_update_key(outfptr, TDOUBLE, "AOGAINH1", (void*)&head->aosb->loop_gain_ho1,"AO loop high order gain group 1",  &status);
      fits_update_key(outfptr, TDOUBLE, "AOGAINH2", (void*)&head->aosb->loop_gain_ho2,"AO loop high order gain group 2",  &status);
      fits_update_key(outfptr, TDOUBLE, "AOMODSP1", (void*)&head->aosb->tt_freq[0],"AO modulation speed (Hz), axis 1",  &status);
      fits_update_key(outfptr, TDOUBLE, "AOMODSP2", (void*)&head->aosb->tt_freq[1],"AO modulation speed (Hz), axis 2",  &status);
      fits_update_key(outfptr, TDOUBLE, "AOMODAM1", (void*)&head->aosb->tt_amp[0],"AO modulation amplitude (millirad), axis 1",  &status);
      fits_update_key(outfptr, TDOUBLE, "AOMODAM2", (void*)&head->aosb->tt_amp[1],"AO modulation amplitude (millirad), axis 2",  &status);
      fits_update_key(outfptr, TDOUBLE, "AOMODOF1", (void*)&head->aosb->tt_offset[0],"AO piezo offset (millirad), axis 1",  &status);
      fits_update_key(outfptr, TDOUBLE, "AOMODOF2", (void*)&head->aosb->tt_offset[1],"AO piezo offset (millirad), axis 2",  &status);
      fits_update_key(outfptr, TDOUBLE, "VFW1POS", (void *)&(head->aosb->filter1_pos),"Position of MagAO Filter Wheel #1",  &status);
      fits_update_key(outfptr, TSTRING, "VFW1POSN", (void *)(head->aosb->filter1_name),"Filter name in MagAO Filter Wheel #1",  &status);
      fits_update_key(outfptr, TDOUBLE, "BAYSIDX", (void *)&(head->aosb->baysidex),"Position of MagAO Bayside X stage",  &status);
      fits_update_key(outfptr, TINT, "BAYSIDXE", (void *)&(head->aosb->baysidex_enabled),"Enable status of MagAO Bayside X stage",  &status);
      fits_update_key(outfptr, TDOUBLE, "BAYSIDY", (void *)&(head->aosb->baysidey),"Position of MagAO Bayside Y stage",  &status);
      fits_update_key(outfptr, TINT, "BAYSIDYE", (void *)&(head->aosb->baysidey_enabled),"Enable status of MagAO Bayside Y stage",  &status);
      fits_update_key(outfptr, TDOUBLE, "BAYSIDZ", (void *)&(head->aosb->baysidez),"Position of MagAO Bayside Z stage",  &status);
      fits_update_key(outfptr, TINT, "BAYSIDZE", (void *)&(head->aosb->baysidez_enabled),"Enable status of MagAO Bayside Z stage",  &status);
   }

   return status;
}

int write_visao_fits_visao_header(fitsfile * outfptr, visao_imheader * head)
{
   struct tm *timestamp_tm;
   char tmp[30], datestr[80];
   int status = 0;
   if(head->csb)
   {
      fits_update_key(outfptr, TINT, "V47WINDX", (void *)&(head->csb->windowx),"CCD47 x window",  &status);
      fits_update_key(outfptr, TINT, "V47WINDY", (void *)&(head->csb->windowy),"CCD47 y window",  &status);
      fits_update_key(outfptr, TINT, "V47BINX", (void *)&(head->csb->xbin),"CCD47 x binning",  &status);
      fits_update_key(outfptr, TINT, "V47BINY", (void *)&(head->csb->xbin),"CCD47 y binning",  &status);
      fits_update_key(outfptr, TINT, "V47PIXRT", (void *)&(head->csb->speed),"CCD47 speed (kHz)",  &status);
      fits_update_key(outfptr, TINT, "V47REPS", (void *)&(head->csb->repetitions),"CCD47 accumulator repetitions",  &status);
      fits_update_key(outfptr, TDOUBLE, "V47FRMRT", (void *)&(head->csb->framerate),"CCD47 frame rate (fps)",  &status);
      double et = 1./head->csb->framerate;
      fits_update_key(outfptr, TDOUBLE, "EXPTIME", (void *)&(et),"CCD47 exposure time (sec)",  &status);
      
      timestamp_tm = gmtime(&head->timestamp_dma.tv_sec);
      strftime(tmp, 30, "%Y-%m-%dT%H:%M:%S", timestamp_tm);
      sprintf(datestr, "%s.%06u", tmp, (unsigned) head->timestamp_dma.tv_usec);
               //yyyy-mm-ddTHH:MM:SS[.sss]
      fits_update_key(outfptr, TSTRING, "FGDMATS", (void *)datestr, "F.G. DMA Timestamp",  &status);
      
      
      switch(head->csb->gain)
      {
         case 0:
            fits_update_key(outfptr, TSTRING, "V47GAIN", (void *)"HIGH","CCD47 gain",  &status);
            break;
         case 1:
            fits_update_key(outfptr, TSTRING, "V47GAIN", (void *)"MED HIGH","CCD47 gain",  &status);
            break;
         case 2:
            fits_update_key(outfptr, TSTRING, "V47GAIN", (void *)"MED LOW","CCD47 gain",  &status);
            break;
         case 3:
            fits_update_key(outfptr, TSTRING, "V47GAIN", (void *)"LOW","CCD47 gain",  &status);
            break;
         default:
            fits_update_key(outfptr, TSTRING, "V47GAIN", (void *)"UNK","CCD47 gain",  &status);
            break;
      }
      fits_update_key(outfptr, TDOUBLE, "V47JTEMP", (void *)&(head->csb->joe_temp),"CCD47 little joe case temp (C)",  &status);
      if(head->vssb) //We want exhaust temp to show up in the right spot.
      {
         fits_update_key(outfptr, TDOUBLE, "V47JXTMP", (void *)&(head->vssb->Joe47Temp),"CCD47 little joe exhaust temp (C)",  &status);
      }
      fits_update_key(outfptr, TDOUBLE, "V47TEMP1", (void *)&(head->csb->head_temp1),"CCD47 head temp (C) #1",  &status);
      fits_update_key(outfptr, TDOUBLE, "V47TEMP2", (void *)&(head->csb->head_temp2),"CCD47 head temp (C) #2",  &status);
      fits_update_key(outfptr, TINT, "V47BLCK1", (void *)&(head->csb->black0),"CCD47 channel #1 black level",  &status);
      fits_update_key(outfptr, TINT, "V47BLCK2", (void *)&(head->csb->black1),"CCD47 channel #2 black level",  &status);
      switch(head->csb->imtype)
      {
         case 0:
            fits_update_key(outfptr, TSTRING, "VIMTYPE", (void *)"SCIENCE","Image Type",  &status);
            break;
         case 1:
            fits_update_key(outfptr, TSTRING, "VIMTYPE", (void *)"ACQUISITION","Image Type",  &status);
            break;
         case 2:
            fits_update_key(outfptr, TSTRING, "VIMTYPE", (void *)"DARK","Image Type",  &status);
            break;
         case 3:
            fits_update_key(outfptr, TSTRING, "VIMTYPE", (void *)"SKY","Image Type",  &status);
            break;
         case 4:
            fits_update_key(outfptr, TSTRING, "VIMTYPE", (void *)"FLAT","Image Type",  &status);
            break;
         default:
            fits_update_key(outfptr, TSTRING, "VIMTYPE", (void *)"UNK","Image Type",  &status);
            break;
      }

   }
   if(head->fsb)
   {
      fits_update_key(outfptr, TDOUBLE, "VFOCPOS", (void *)&(head->fsb->cur_pos),"Position of VisAO Focus stage (microns)",  &status);
      if(head->fsb->is_moving)
         fits_update_key(outfptr, TSTRING, "VFOCSTAT", (void *)"MOVING","Status of VisAO Focus stage",  &status);
      else
         fits_update_key(outfptr, TSTRING, "VFOCSTAT", (void *)"STOPPED","Status of VisAO Focus stage",  &status);
      if(head->fsb->power_state)
         fits_update_key(outfptr, TSTRING, "VFOCPWR", (void *)"ON","Status of VisAO Focus stage controller",  &status);
      else
         fits_update_key(outfptr, TSTRING, "VFOCPWR", (void *)"OFF","Status of VisAO Focus stage controller",  &status);
   }
   if(head->fw2sb)
   {
      fits_update_key(outfptr, TDOUBLE, "VFW2POS", (void *)&(head->fw2sb->pos),"Position of VisAO Filter Wheel #2",  &status);
      fits_update_key(outfptr, TSTRING, "VFW2POSN", (void *)(head->fw2sb->filter_name),"Filter name in VisAO Filter Wheel #2",  &status);
   }
   if(head->fw3sb)
   {
      fits_update_key(outfptr, TDOUBLE, "VFW3POS", (void *)&(head->fw3sb->pos),"Position of VisAO Filter Wheel #3",  &status);
      fits_update_key(outfptr, TSTRING, "VFW3POSN", (void *)(head->fw3sb->filter_name),"Filter name in VisAO Filter Wheel #3",  &status);
   }
   if(head->wsb)
   {
      if(head->wsb->cur_pos == 1) fits_update_key(outfptr, TSTRING, "VWOLLSTN", (void *)"IN","Status of VisAO Wollaston",  &status);
      if(head->wsb->cur_pos == 0) fits_update_key(outfptr, TSTRING, "VWOLLSTN", (void *)"INTERMEDIATE","Status of VisAO Wollaston",  &status);
      if(head->wsb->cur_pos == -1) fits_update_key(outfptr, TSTRING, "VWOLLSTN", (void *)"OUT","Status of VisAO Wollaston",  &status);
   }

   if(head->ssb)
   {
      int sps = head->ssb->power_state;
      char ssat[5], hsat[10];

      if(head->ssb->sync_enabled == 0) strcpy(hsat, "IGNORED");
      else hsat[0] = '\0';

      if( sps == 0) strcpy(ssat, "OFF"); 
      else strcpy(ssat, "UNK");
        if (sps == -1) strcpy(ssat, "UNK");
      else
      {
         if(head->ssb->in_auto) strcpy(ssat, "AUTO");
         else
         {
            if(head->ssb->sw_state == 1) strcpy(ssat, "OPEN");
            else if(head->ssb->sw_state == -1) strcpy(ssat, "SHUT");
            else strcpy(ssat, "INT");
            if(head->ssb->sync_enabled == 1)
            {
               if(head->ssb->hw_state == 1) strcpy(hsat, "OPEN");
               else if(head->ssb->hw_state == -1) strcpy(hsat, "SHUT");
               else strcpy(hsat, "INT");
            }
            
         }
      }
      //We'll update for more than OFF or UNK 
      fits_update_key(outfptr, TSTRING, "VSHUTTER", (void *)ssat,"Status of VisAO Shutter",  &status);
      
      if(hsat[0]) fits_update_key(outfptr, TSTRING, "VSHUTSYN", (void *)hsat,"Status of VisAO Shutter Sync Signal",  &status);
      else fits_update_key(outfptr, TSTRING, "VSHUTSYN", (void *)"UNK","Status of VisAO Shutter Sync Signal",  &status);
   }
   if(head->gsb)
   {
      if(head->gsb->power)
         fits_update_key(outfptr, TSTRING, "VGIMPWR", (void *)"ON","Status of VisAO Gimbal controller",  &status);
      else
         fits_update_key(outfptr, TSTRING, "VGIMPWR", (void *)"OFF","Status of VisAO Gimbal controller",  &status);

      
      fits_update_key(outfptr, TDOUBLE, "VGIMXPOS", (void *)&(head->gsb->xpos),"X Position of VisAO Gimbal",  &status);

      fits_update_key(outfptr, TDOUBLE, "VGIMYPOS", (void *)&(head->gsb->ypos),"Y Position of VisAO Gimbal",  &status);
   }

   if(head->zsb)
   {
      if(head->zsb->state == STATE_OFF)
      {
         fits_update_key(outfptr, TSTRING, "VFSSTAT", (void *)"OFF","Status of VisAO Field Stop Stage",  &status);
         fits_update_key(outfptr, TSTRING, "VFSPOS", (void *)"-1","Position of VisAO Field Stop Stage",  &status);
      }
      else if(head->zsb->state == STATE_HOMING)
      {
         fits_update_key(outfptr, TSTRING, "VFSSTAT", (void *)"HOMING","Status of VisAO Field Stop Stage",  &status);
         fits_update_key(outfptr, TSTRING, "VFSPOS", (void *)"-1","Position of VisAO Field Stop Stage",  &status);
      }
      else if(head->zsb->state == STATE_READY || head->zsb->state == STATE_OPERATING)
      {
         if(head->zsb->state == STATE_READY)
         {
            fits_update_key(outfptr, TSTRING, "VFSSTAT", (void *)"STOPPED","Status of VisAO Field Stop Stage",  &status);
         }
         else if(head->zsb->state == STATE_OPERATING)
         {
            fits_update_key(outfptr, TSTRING, "VFSSTAT", (void *)"MOVING","Status of VisAO Field Stop Stage",  &status);
         }
         fits_update_key(outfptr, TDOUBLE, "VFSPOS", (void *)&(head->zsb->pos),"Position of VisAO Field Stop Stage",  &status);
      }
      else
      {
         fits_update_key(outfptr, TSTRING, "VFSSTAT", (void *)"UNKNOWN","Status of VisAO Field Stop Stage",  &status);
         fits_update_key(outfptr, TSTRING, "VFSPOS", (void *)"-1","Position of VisAO Field Stop Stage",  &status);
      }
   }
   else
   {
      fits_update_key(outfptr, TSTRING, "VFSSTAT", (void *)"UKNOWN","Status of VisAO Field Stop Stage",  &status);
      fits_update_key(outfptr, TSTRING, "VFSPOS", (void *)"-1","Position of VisAO Field Stop Stage",  &status);
   }
     
   if(head->hwpsb)
   {
      if(head->hwpsb->state == STATE_OFF)
      {
         fits_update_key(outfptr, TSTRING, "VHWPSTAT", (void *)"OFF","Status of VisAO Half Wave Plate",  &status);
         fits_update_key(outfptr, TSTRING, "VHWPPOS", (void *)"-1","Position of VisAO Half Wave Plate",  &status);
      }
      else if(head->hwpsb->state == STATE_HOMING)
      {
         fits_update_key(outfptr, TSTRING, "VHWPSTAT", (void *)"HOMING","Status of VisAO Half Wave Plate",  &status);
         fits_update_key(outfptr, TSTRING, "VHWPPOS", (void *)"-1","Position of VisAO Half Wave Plate",  &status);
      }
      else if(head->hwpsb->state == STATE_READY || head->hwpsb->state == STATE_OPERATING || head->hwpsb->state == STATE_BUSY)
      {
         if(head->hwpsb->state == STATE_READY)
         {
            fits_update_key(outfptr, TSTRING, "VHWPSTAT", (void *)"STOPPED","Status of VisAO Half Wave Plate",  &status);
         }
         else if(head->hwpsb->state == STATE_OPERATING)
         {
            fits_update_key(outfptr, TSTRING, "VHWPSTAT", (void *)"MOVING","Status of VisAO Half Wave Plate",  &status);
         }
         else if(head->hwpsb->state == STATE_BUSY)
         {
            fits_update_key(outfptr, TSTRING, "VHWPSTAT", (void *)"CONTINUOUS","Status of VisAO Half Wave Plate",  &status);
         }
         fits_update_key(outfptr, TDOUBLE, "VHWPPOS", (void *)&(head->hwpsb->pos),"Position of VisAO Half Wave Plate",  &status);
      }
      else
      {
         fits_update_key(outfptr, TSTRING, "VHWPSTAT", (void *)"UNKNOWN","Status of VisAO Half Wave Plate",  &status);
         fits_update_key(outfptr, TSTRING, "VHWPPOS", (void *)"-1","Position of VisAO Half Wave Plate",  &status);
      }
   }
   else
   {
      fits_update_key(outfptr, TSTRING, "VHWPSTAT", (void *)"UKNOWN","Status of VisAO Half Wave Plate",  &status);
      fits_update_key(outfptr, TSTRING, "VHWPPOS", (void *)"-1","Position of VisAO Half Wave Plate",  &status);
   }
  /* if(head->rsb)
   {
      double tmp = head->rsb->avgwfe_1_sec;
      fits_update_key(outfptr, TDOUBLE, "WFEAVG", (void *)(&tmp), "1 sec average WFE, nm rms phase",  &status);
      tmp = head->rsb->stdwfe_1_sec;
      fits_update_key(outfptr, TDOUBLE, "WFESTD", (void *)(&tmp), "1 sec std dev of WFE, nm rms phase",  &status);
      tmp = head->rsb->inst_wfe;
      //fits_update_key(outfptr, TDOUBLE, "WFEINST", (void *)(&tmp), "Instantaneous WFE, nm rms phase",  &status);
}*/
      
   return status;
}

template <> int visao_fits_create_img<short>( fitsfile *fptr, int naxis, long *naxes, int *status)
{
   return fits_create_img( fptr, SHORT_IMG, naxis, naxes, status);
}

template <> int visao_fits_create_img<unsigned char>( fitsfile *fptr, int naxis, long *naxes, int *status)
{
   return fits_create_img( fptr, BYTE_IMG, naxis, naxes, status);
}

template <> int visao_fits_write_subset<short>(fitsfile *fptr, long *fpixel, long *lpixel, short *array,  int *status)
{
   return fits_write_subset(fptr,TSHORT , fpixel, lpixel, array, status);
}

template <> int visao_fits_write_subset<unsigned char>(fitsfile *fptr, long *fpixel, long *lpixel, unsigned char *array,  int *status)
{
   return fits_write_subset(fptr,TBYTE, fpixel, lpixel, array, status);
}

int write_visao_raw(const char * name_base, sharedimS *sim, visao_imheader * head __attribute__((unused)))
{
   FILE *fout;
   std::string fname;
   char fext[21];
   fname = name_base;
   get_visao_filename(fext, &sim->frame_time);
   
   fname += fext;
   fname += ".raw";
   
   fout = fopen(fname.c_str(), "w+");
   if(fout ==0 )
   {
      std::cerr << "Error opening file " << fname << "\n";
      return -1;
   }
   fwrite(sim->imdata, sizeof(short), sim->nx*sim->ny, fout);
   
   fclose(fout);
   return 0;
}

int get_fits_im(unsigned char *im, long hduno, long *fpix, long *lpix, fitsfile **fptr, const char *fname)
{
   int fstatus = 0;
   long inc[2] = {1,1};
   //std::cout << fname << "\n" << hduno << "\n";
   if(*fptr == 0 && fname > 0)
   {
      fits_open_file(fptr, fname, READONLY, &fstatus);
      if (fstatus)
      {
         fprintf(stderr, "Error in get_fits_data.\n");
         fits_report_error(stderr, fstatus); /* print any error message */
         return -1;
      }
   }
   fits_movabs_hdu(*fptr, hduno, NULL, &fstatus);
   if (fstatus)
   {
      fprintf(stderr, "Error in get_fits_data.\n");
      fits_report_error(stderr, fstatus); /* print any error message */
      return -1;
   }
   
   
   fits_read_subset(*fptr, TBYTE, fpix, lpix, inc, 0, im, 0, &fstatus);
   if (fstatus && fstatus != 107)
   {
      fprintf(stderr, "Error in get_fits_data.\n");
      fits_report_error(stderr, fstatus); /* print any error message */
      return -1;
   }
   
   return fstatus;
}

int get_fits_im(float *im, long hduno, long *fpix, long *lpix, fitsfile **fptr, const char *fname)
{
   int fstatus = 0;
   long inc[2] = {1,1};
   //std::cout << fname << "\n" << hduno << "\n";
   if(*fptr == 0 && fname > 0)
   {
      fits_open_file(fptr, fname, READONLY, &fstatus);
      if (fstatus)
      {
         fprintf(stderr, "Error in get_fits_data.\n");
         fits_report_error(stderr, fstatus); // print any error message
         return -1;
      }
   }
   
   
   
   fits_movabs_hdu(*fptr, hduno, NULL, &fstatus);
   if (fstatus)
   {
      fprintf(stderr, "Error in get_fits_data.\n");
      fits_report_error(stderr, fstatus); // print any error message
      return -1;
   }
   
   std::cout << fpix[0] << " " << fpix[1] << std::endl;
   std::cout << lpix[0] << " " << lpix[1] << std::endl;
   std::cout << inc[0] << " " << inc[1] << std::endl;

   fits_read_subset(*fptr, TFLOAT, fpix, lpix, inc, 0, im, 0, &fstatus);
   if (fstatus && fstatus != 107)
   {
      fprintf(stderr, "Error in get_fits_data.\n");
      fits_report_error(stderr, fstatus); // print any error message
      return -1;
   }
   
   return fstatus;
}

int get_fits_im(double *im, long hduno, long *fpix, long *lpix, fitsfile **fptr, const char *fname)
{
   int fstatus = 0;
   long inc[2] = {1,1};
   //std::cout << fname << "\n" << hduno << "\n";
   if(*fptr == 0 && fname > 0)
   {
      fits_open_file(fptr, fname, READONLY, &fstatus);
      if (fstatus) 
      {
         fprintf(stderr, "Error in get_fits_data.\n");
         fits_report_error(stderr, fstatus); // print any error message 
         return -1;
      }
   }

  
   fits_movabs_hdu(*fptr, hduno, NULL, &fstatus);
   if (fstatus) 
   {
      fprintf(stderr, "Error in get_fits_data.\n");
      fits_report_error(stderr, fstatus); // print any error message 
      return -1;
   }
   
   
   fits_read_subset(*fptr, TDOUBLE, fpix, lpix, inc, 0, im, 0, &fstatus);
   if (fstatus && fstatus != 107) 
   {
      fprintf(stderr, "Error in get_fits_data.\n");
      fits_report_error(stderr, fstatus); // print any error message 
      return -1;
   }
   
   return fstatus;
}

