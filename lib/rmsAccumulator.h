

#ifndef __rmsAccumulator_h__
#define __rmsAccumulator_h__

#include <iostream>
#include <vector>
#include <cmath>

template<class floatT>
class rmsAccumulator
{
public:
   std::vector<std::vector<floatT> >  values;
   std::vector<floatT> means;
   std::vector<floatT> rmss;
   
   int length;
   int nStreams;
   
   void resize(int nstr, int len);
 
   int currVal;
   
   floatT minLim;
   floatT maxLim;

   void addValues(std::vector<floatT> newVals);


   void addValues(floatT * newVals);
   
   void calcMeans();
    
   void calcRmss();

};

template<typename floatT>
void rmsAccumulator<floatT>::resize(int nstr, int len)
{
   nStreams = nstr;
   length=len;
   
   values.resize(length);
   
   for(int i=0;i<length; ++i)
   {
      values[i].resize(nStreams, 0);
   }

   means.resize(nStreams,0);
   rmss.resize(nStreams,0);

   
   currVal = 0;
}
 
template<typename floatT>
void rmsAccumulator<floatT>::addValues(std::vector<floatT> newVals)
{
   if(newVals.size() != values[0].size())
   {
      std::cerr << "wrong size\n";
      return;
   }
   
   values[currVal] = newVals;
   
   ++currVal;
   
   if(currVal >= values.size()) currVal = 0;
   
}

template<typename floatT>
void rmsAccumulator<floatT>::addValues(floatT * newVals)
{
   for(int i=0;i<nStreams;++i)
      values[currVal][i] = newVals[i];
   
   ++currVal;
   
   if(currVal >= values.size()) currVal = 0;
   
}
   
template<typename floatT>
void rmsAccumulator<floatT>::calcMeans()
{
   for(int i=0;i<nStreams;++i)
   {
      means[i] = values[currVal][i]/length;
   }
      
   int j = currVal + 1;
   if(j >= length) j = 0;
   while(j != currVal)
   {  
      for(int i=0;i<nStreams;++i)
      {
         means[i] += values[j][i]/length;
      }  
      ++j;
      if(j >= length) j = 0;
   }
}
    
template<typename floatT>
void rmsAccumulator<floatT>::calcRmss()
{
   floatT d;
   for(int i=0;i<nStreams;++i)
   {
      d = values[currVal][i];
      if(fabs(d) < 1e3)
         rmss[i] = (d*d)/length;
      else
         rmss[i] = 0;
   }
      
   int j = currVal + 1;
   if(j >= length) j = 0;
   while(j != currVal)
   {
      
      for(int i=0;i<nStreams;++i)
      {
         d = values[j][i];
         if(fabs(d)<1e3)
             rmss[i] += (d*d)/length;
      }  
      ++j;
      if(j >= length) j = 0;
   }
}

#endif //__rmsAccumulator_h__
