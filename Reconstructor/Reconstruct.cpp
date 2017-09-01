
#include <iostream>
#include "svbksb.h"
#include <fstream>
#include <cmath>

#include "time.h"
#include "sys/time.h"

#define GSL_RANGE_CHECK_OFF
#include "gsl/gsl_matrix.h"
#include "gsl/gsl_linalg.h"
#include "gsl/gsl_blas.h"

#include "fitsio.h"
#include "../../Magellan/lib/bcu_diag.h"

#include "mx/fileUtils.hpp"

#define DATAT float

#include "recmat.h"

#include "fir_filter.h"

int reconstruct_bcu39frames(std::string recmatname, std::string subdir, int outMode=0) //std::string flist, int nframes)
{
   //std::string fname;
   
   fitsfile * fptr1 = 0;

   std::vector<std::string> flist = mx::getFileNames(subdir, "BCU39", ".fits"); 
   sort(flist.begin(), flist.end());
  
   int fstatus;
   
   long fpix[2]={1,1}, lpix[2];

   recmat rmat(recmatname);
   rmat.rec_tech = REC_ATLAS;
   //rmat.rec_tech = REC_GPU;
   
   
   std::vector<std::ofstream *> outs;
   
   if(outMode == 1)
   {
      outs.resize(rmat.n_modes);
      
      std::stringstream fn;
      for(int i=0;i<rmat.n_modes;i++)
      {
         fn.str("");
         fn << "mode_" << i << ".dat";
         outs[i] = new std::ofstream;
         outs[i]->open(fn.str().c_str());
      }
   }
   
   
   
   
   
   
   
   
   
   
   lpix[0] = sizeof(slopecomp_diagframe_pixels_slopes);
   lpix[1] = 1;
   
   unsigned char * im39 = new unsigned char[sizeof(slopecomp_diagframe_pixels_slopes)];

   float *slopes = (float *) (im39+12832);
   
   float sumvar, ho1, ho2;
   double t0, t1, avdt = 0, avdtrec = 0;

   float * svs = new float[flist.size()];
   float sr, filtsv;
   uint32 frameno;
   
   for(int i=0;i<flist.size();i++)
   {
      //fin >> fname;
      //fname = flist[i];
      
      get_fits_im(im39, 1, fpix, lpix, &fptr1, flist[i].c_str());
      fits_close_file(fptr1, &fstatus);
      fptr1 = 0;

      frameno = *( (uint32*) im39);
      
      t0 = get_curr_time();

      rmat.reconstruct(slopes);
      
      avdtrec += get_curr_time() - t0;

      rmat.calc_sumvar(&sumvar, 0, -1, 0);
      rmat.calc_sumvar(&ho1, 2, 200,0);
      rmat.calc_sumvar(&ho2, 200,-1, 0);
      
      
      t1 = get_curr_time();
      
      avdt += (t1-t0);
      
      //std::cout << frameno << " " << sumvar << " " << rmat.unit_conversion*rmat.reflection_gain*rmat.amp[0] << " " << rmat.unit_conversion*rmat.reflection_gain*rmat.amp[1] << "\n";
      
      if(outMode == 1)
      {
         for(int j=0;j<rmat.n_modes;++j)
         {
            (*outs[j]) << frameno << " " << rmat.amp[j] << "\n";
         }
      }
      else
      {
         std::cout << frameno << " " << rmat.unit_conversion*rmat.reflection_gain*rmat.amp[0] << " " << rmat.unit_conversion*rmat.reflection_gain*rmat.amp[1] << " " <<
         sqrt(ho1) << " " << 
         sqrt(ho2) << " " << 
         rmat.unit_conversion*rmat.reflection_gain*rmat.amp[100] << " " << rmat.unit_conversion*rmat.reflection_gain*rmat.amp[150] << "\n";
      }
   }
      

   std::cerr << "Avg Rec Time: " << avdtrec/flist.size() << "\n";
   std::cerr << "Avg Tot Time: " << avdt/flist.size() << "\n";
   
   if(outMode == 1)
   {
      for(int j=0;j<rmat.n_modes;++j)
      {
         outs[j]->close();
         delete outs[j];
      }
   }
   return 0;
}


int make_slopeims(std::string olist, std::string flist, int nframes)
{
   std::ifstream fin;
   std::ifstream ofin; //The output file name

   std::ifstream lutxin;
   std::ifstream lutyin;
   
   std::string fname;

   fitsfile * fptr1 = 0;
   fitsfile * fptr2 = 0;

   //uint32 * frameno;
   
   //VisAO::fir_filter fir;
   //fir.read_coef_file("filters/FIR_250Hz_GERMO_10_20_1_20.txt");
   
   int fstatus;
   //int naxis;
   long fpix[2]={1,1}, lpix[2];

   long naxes[3], out_fpix[3], out_lpix[3];//,fpix[2];
   naxes[0] = 80;
   naxes[1] = 40;
   naxes[2] = nframes;
   out_fpix[0] = 1;
   out_fpix[1] = 1;
   out_lpix[0] = 80;
   out_lpix[1] = 40;
   
   //The slope image
   float * frame = new float[6400];
   for(int i=0;i<6400;i++) frame[i] = 0;
   
   //Create and read in the lookup table.
   int lut_x[6400];
   int lut_y[6400];
   
      
   fname = getenv("VISAO_ROOT");
   fname += "/calib/ccd39/slopex";
   //std::cout << fname << "\n";
   lutxin.open(fname.c_str());
   
   fname = getenv("VISAO_ROOT");
   fname += "/calib/ccd39/slopey";
   lutyin.open(fname.c_str());
   int xi;
   for(int i=0;i<6400;i++)
   {
      //std::cout << lut_x[i] << "\n";
      lutxin >> lut_x[i];//xi;
      
      lutyin >> lut_y[i];
   }
   lutxin.close();
   lutyin.close();
   
   
   lpix[0] = sizeof(slopecomp_diagframe_pixels_slopes);
   lpix[1] = 1;
   
   unsigned char * im39 = new unsigned char[sizeof(slopecomp_diagframe_pixels_slopes)];

   float *slopes = (float *) (im39+12832);
   
   //gsl_vector_float_view gslSlopes = gsl_vector_float_view_array(slopes, rmat.n_slopes);
   //gsl_vector_float_view gslAmp = gsl_vector_float_view_array(rmat.amp, rmat.n_modes);

   
   fin.open(flist.c_str());

   float sumvar;
   double t0, t1, avdt = 0, avdtrec = 0;

   float * svs = new float[nframes];
   float sr, filtsv;
   
      fits_create_file(&fptr2, olist.c_str(), &fstatus);
      fits_create_img( fptr2, FLOAT_IMG, 3, naxes, &fstatus);
   
   
   for(int i=0;i<nframes;i++)
   {
      fin >> fname;
      //std::cout << fname << "\n";
      get_fits_im(im39, 1, fpix, lpix, &fptr1, fname.c_str());
      fits_close_file(fptr1, &fstatus);
      fptr1 = 0;

      std::cout << *( (uint32*) im39)  << "\n";
      for(int j=0;j<6400;j++)
      {
         if(lut_x[j] != -1) 
         {
            frame[j] = slopes[lut_x[j]];
         }
         //else frame[j] = 0;
         
         //if(lut_y[j] != -1) 
         //{
            //frame[3200 + j] = slopes[1344 + lut_y[j]];
         //}
         //else frame[3200 + j] = 0;
         
         
      }

      out_fpix[2] = i+1;
      out_lpix[2] = i+1;
      fits_write_subset(fptr2,TFLOAT , out_fpix, out_lpix, &frame[3200], &fstatus);
      
   }

         fits_close_file(fptr2, &fstatus);

   
   fin.close();
   return 0;
}

int get_frameno(std::string flist, int nframes)
{
   std::ifstream fin;
 
   
   std::string fname;

   fitsfile * fptr1 = 0;
   
   int fstatus;
   //int naxis;
   long fpix[2]={1,1}, lpix[2];
  
   lpix[0] = sizeof(slopecomp_diagframe_pixels_slopes);
   lpix[1] = 1;
   
   unsigned char * im39 = new unsigned char[sizeof(slopecomp_diagframe_pixels_slopes)];
   
   fin.open(flist.c_str());
   
   for(int i=0;i<nframes;i++)
   {
      fin >> fname;
      //std::cout << fname << "\n";
      get_fits_im(im39, 1, fpix, lpix, &fptr1, fname.c_str());
      fits_close_file(fptr1, &fstatus);
      fptr1 = 0;

      std::cout << *( (uint32*) im39)  << "\n";   
   }

   fin.close();
   return 0;
}

int TimeToDie;
fifo_list * global_fifo_list;

int main(int argc, char**argv)
{
   uid_t euid_real; ///< The real user id of the proces
   uid_t euid_called; ///< The user id of the process as called (that is when the constructor gets called).
   uid_t suid; ///< The save-set user id of the process
   
   getresuid(&euid_real, &euid_called, &suid);
   
   
   int rv;
   struct sched_param schedpar;
   
   schedpar.sched_priority = 90;
   
   errno = 0;
   
   rv = sched_setscheduler(0, SCHED_FIFO, &schedpar);

   if(rv < 0)
   {
      std::cerr << "Setting scheduler priority to " << 90 << " failed.  Errno says: " << strerror(errno) << ".\n";
   }
   else
   {
      std::cerr << "Scheduler priority (RT_priority) set to " << 90 << ".\n";
   }
   
   //reconstruct_bcu39frames("/home/aosup/RecMats/Rec_20111125_165625.fits", "/home/aosup/visao/data/archive/ccd39/2011.11.28/even_fainter/SRTS_wdisturb/bcu39.list", 13508);

   std::string recstr, flist;
   int nframes;
   
   recstr = argv[1];
   if(argc == 3)
   {
      flist = argv[2];
   }
   else
   { 
      flist = "./";
   }
   //nframes = atoi(argv[3]);
   
   std::cerr << recstr << "\n";
   std::cerr << flist << "\n";
   
   reconstruct_bcu39frames(recstr, flist,1);
   
   //make_slopeims("olist.fits", flist, nframes);
   
   //get_frameno(flist, nframes);
   
   return 0;
}
   
   /*
   DATAT **U;
   DATAT **V;
   DATAT *S;
  
   DATAT **slopes;
   DATAT *vars;
   
   U = new DATAT*[1096];
   for(int i=0;i<1096; i++) U[i] = new DATAT[412];

   V = new DATAT*[412];
   for(int i=0;i<412; i++) V[i] = new DATAT[412];

   S = new DATAT[412];

   slopes = new DATAT*[2550];
   for(int i=0;i<2550;i++) slopes[i] = new DATAT[1096];

   vars = new DATAT[412];

   gsl_matrix *gslU, *gslV;
   gsl_vector *gslS, *gslwork;
   gslU = gsl_matrix_alloc(1096, 412);
   gslV = gsl_matrix_alloc(412,412);
   gslS = gsl_vector_alloc(412);
   gslwork = gsl_vector_alloc(412);

   std::ifstream fin;
   fin.open("data/caos_matint412_matint.txt");
   int n;
   fin >> n;
   fin >> n;
   for(int i=0;i<1096;i++) for(int j=0;j<412;j++) fin >> *gsl_matrix_ptr(gslU, i,j);
   fin.close();
   
   gsl_linalg_SV_decomp(gslU, gslV, gslS, gslwork);
   
   for(int i=0;i<1096;i++) for(int j=0;j<412;j++) U[i][j] = gsl_matrix_get(gslU, i, j);
   for(int i=0;i<412;i++) for(int j=0;j<412;j++) V[i][j] = gsl_matrix_get(gslV, i, j);
   for(int i=0;i<412;i++) S[i] = gsl_vector_get(gslS, i);

   gsl_matrix_free(gslU);
   gsl_matrix_free(gslV);
   gsl_vector_free(gslS);
   gsl_vector_free(gslwork);
   
   //std::ifstream fin;
//    fin.open("data/caos_matint412_U.txt");
//    
//    for(int i=0;i<1096;i++) for(int j=0;j<412;j++) fin >> U[i][j];
//    fin.close();
// 
//    for(int i=0; i<10;i++)
//    std::cout << gsl_matrix_get(gslU, 0, i) << " " << U[0][i] << "\n";
//    
//    fin.open("data/caos_matint412_V.txt");
//    
//    for(int i=0;i<412;i++) for(int j=0;j<412;j++) fin >> V[i][j];
//    fin.close();
// 
//    std::cout << "\nV:\n";
//    for(int i=0; i<10;i++)
//       std::cout << gsl_matrix_get(gslV, 0, i) << " " << V[0][i] << "\n";
//    
//    
//    fin.open("data/caos_matint412_S.txt");
//    
//    for(int i=0;i<412;i++) fin >> S[i];
//    fin.close();
// 
//    std::cout << "\nS_S:\n";
//    for(int i=0; i<10;i++)
//       std::cout << gsl_vector_get(gslS, i) << " " << S[i] << "\n";
//   
//    
//    exit(0);

   for(int i=197;i<412;i++) S[i] =0;
   
   fin.open("data/slopes_from_pyr.txt");
  
   for(int i=0;i<2550;i++) for(int j=0;j<1096;j++) fin >> slopes[i][j];

   fin.close();

   fin.open("data/caos_mirdef412_vars.txt");
   fin >> n;
   for(int i=0;i<412;i++) fin >> vars[i];
   fin.close();
   
   preconditionU_F(U, S, 1096, 412);

   //preconditionU(U, S, 1096, 412);
   
   DATAT *s = new DATAT[2550];
   DATAT *tmp = new DATAT[412];
   DATAT *amp = new DATAT[412];
   DATAT a = .9;
   DATAT b = -.23;
   DATAT lamgain = pow(2.*3.14159/700.,2);
   DATAT sumvar;
   
   double t0, t1;
   int i;
   t0 = get_curr_time();
   for(i=0; i< 2550; i++)
   {
      //backsub_precond_F(U, V, 1096, 412, slopes[i], amp, tmp);
      backsub_precond_F_threads(U, V, 1096, 412, slopes[i], amp, tmp,2);
      //backsub_precond(U, V, 1096, 412, slopes[i], amp, tmp);
      sumvar = 0.0;
      for(int j=2;j<412;j++)
      {
         sumvar +=  4.*amp[j]*amp[j]*vars[j];
      }

      s[i] = a*exp(-1.*lamgain*sumvar) + b;
   }
   t1 = get_curr_time();

   std::cerr.precision(10);
   
   std::cerr << (t1-t0)/((double) i) << "\n";
   //for(int i=0;i<412;i++) std::cout << amp[i] << "\n";
   for(int j=0;j<2550;j++) std::cout << s[j] << "\n";
   return 0;
}*/


      
   
