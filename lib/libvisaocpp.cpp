/************************************************************
*    libvisaocpp.cpp
*
* Author: Jared R. Males (jrmales@as.arizona.edu)
*
* VisAO software utilitites, definitions (c++)
*
************************************************************/

/** \file libvisaocpp.cpp
  * \author Jared R. Males
  * \brief VisAO software utilitites, definitions (c++)
  * 
*/

#include "libvisao.h"

int visao_mdstat(char dstat[SYS_N_LOGDRV])
{
   std::ifstream fin;
   char mdstat[1024];

   fin.open("/proc/mdstat");

   if(!fin.good())
   {
      for(int i=0;i<4;i++) dstat[i] = '?';
      return -1;
   }

   fin.read(mdstat, 1024);

   mdstat[fin.gcount()] = 0;

   fin.close();

   std::string mdstr = mdstat;

   int pos = mdstr.find("] [");

   if(pos < 0)
   {
      for(int i=0;i<SYS_N_LOGDRV;i++) dstat[i] = '?';
      return -1;
   }

   pos = mdstr.find("] [", pos+3);
   if(pos < 0)
   {
      for(int i=0;i<SYS_N_LOGDRV;i++) dstat[i] = '?';
      return -1;
   }

   dstat[0] = mdstr[pos+3];
   dstat[1] = mdstr[pos+4];
   
   dstat[2] = '?'; //status of raid0 swap is not checked.
   dstat[3] = '?';
   pos = mdstr.find("] [", pos+3);
   if(pos < 0)
   {
      for(int i=0;i<SYS_N_LOGDRV;i++) dstat[i] = '?';
      return -1;
   }

   dstat[4] = mdstr[pos+3];
   dstat[5] = mdstr[pos+4];

   return 0;
}

int save_preset(std::string calname, double fw1pos, int wollstat, double fw2pos, double fw3pos, std::vector<double> *vec)
{
   std::ofstream fout;
   std::ostringstream fname;

   if(wollstat == 0)
   {
      std::cerr << "Wollaston intermediate.\n";
      return -1;
   }
   if(wollstat < 0) wollstat = 0;

   fname << getenv("VISAO_ROOT") << "/calib/visao/" << calname << "/presets/";
   fname << (int)(fw1pos + .5) << wollstat << (int)(fw2pos + .5) << (int)(fw3pos + .5) << ".preset";
   
   std::cout << fname.str() << "\n";
   
   fout.open(fname.str().c_str(), std::ofstream::trunc);
   
   if(!fout.good())
   {
      std::cerr << "Preset " << fname.str() << " can't be created.\n";
      return -1;
   }
   
   for(unsigned int i=0;i<vec->size();i++)
   {
      fout << (*vec)[i];
      fout << " ";
   }
   fout << "\n";

   fout.close();
   
   return 0;
}

int get_focuscal(double * fcal)
{
   std::string presetf;
   
   std::ifstream fin;
   std::ostringstream fname;


   fname << getenv("VISAO_ROOT") << "/calib/visao/focus/";
   fname << "focus.cal";
   
   presetf = fname.str();
   //std::cout << fname.str() << "\n";
   
   fin.open(fname.str().c_str());
   
   if(!fin.good())
   {
      std::cerr << "Preset " << fname.str() << " not found.\n";
      return -1;
   }
   
   fin >> (*fcal);
   
   fin.close();
   
   
   return 0;
}

int get_preset(std::string calname, double fw1pos, int wollstat, double fw2pos, double fw3pos, std::vector<double> *vec, std::string & presetf)
{

   std::ifstream fin;
   std::ostringstream fname;

   if(wollstat == 0)
   {
      std::cerr << "Wollaston intermediate.\n";
      return -1;
   }
   if(wollstat < 0) wollstat = 0;

   fname << getenv("VISAO_ROOT") << "/calib/visao/" << calname << "/presets/";
   fname << (int)(fw1pos + .5) << wollstat << (int)(fw2pos + .5) << (int)(fw3pos + .5) << ".preset";
   
   presetf = fname.str();
   //std::cout << fname.str() << "\n";
   
   fin.open(fname.str().c_str());
   
   if(!fin.good())
   {
      std::cerr << "Preset " << fname.str() << " not found.\n";
      return -1;
   }
   
   for(unsigned int i=0;i<vec->size();i++)
   {
      fin >> (*vec)[i];
   }
   
   fin.close();
   
   
   return 0;
}

