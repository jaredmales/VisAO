#include "rmsAccumulator.h"


      

// int main()
// {
//    rmsAccumulator<float> rms;
//    
//    rms.resize(18,1000);
//    
//    std::vector<float> add(18,1);
//    
//    //for(int i=0;i<5;++i) std::cout << add[i] << " ";
//    //std::cout << "\n";
//    
//    for(int i=0; i<2000;++i)
//    {
//       rms.calcMeans();
//       rms.calcRmss();
// //       
//       
// 
//       for(int j=0;j<rms.values.size();++j)
//       {
//          for(int k=0;k<rms.values[0].size();++k)
//          {
//             std::cout << rms.values[j][k] << " ";
//          }
//          std::cout << "\n";
//       }
//       std::cout << "-----------------\n";
// //       
//       for(int j=0;j<rms.values[0].size();++j)
//       {
//          std::cout << rms.means[j] << " ";
//       }
//       std::cout<<"\n";
//       for(int j=0;j<rms.values[0].size();++j)
//       {
//          std::cout << rms.rmss[j] << " ";
//       }
//       std::cout << "\n----------------\n";
//       
//       rms.addValues(add);
// 
//    }
//    
// }