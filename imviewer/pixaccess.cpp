#include "pixaccess.h"


float (*getPixPointer(int imv_type))(void*, size_t)
{
   switch(imv_type)
   {
      case IMV_CHAR:
         return &getPix<char>;
      case IMV_UCHAR:
         return &getPix<unsigned char>;
      case IMV_SHORT:
         return &getPix<short>;          
      case IMV_USHORT:
         return &getPix<unsigned short>;         
      case IMV_INT:
         return &getPix<int>;
      case IMV_UINT:
         return &getPix<unsigned int>;           
      case IMV_LONG:
         return &getPix<long>;           
      case IMV_ULONG:
         return &getPix<unsigned long>;          
      case IMV_LONGLONG:
         return &getPix<long long>;       
      case IMV_ULONGLONG:
         return &getPix<unsigned long long>;      
      case IMV_FLOAT:
         return &getPix<float>;          
      case IMV_DOUBLE:
         return &getPix<double>;         
      case IMV_LONGDOUBLE:
         return &getPix<long double>;     
//       case IMV_CMPLXFLOAT:
//          return &getPix<std::complex<float> >;     
//       case IMV_CMPLXDOUBLE:
//          return &getPix<std::complex<double> >;    
//       case IMV_CMPLXLONGDOUBLE:
//          return &getPix<std::complex<long double> >;
      default:
         return 0;
   }
   
   return 0;
}


size_t sizeof_imv_type(int imv_type)
{
   switch(imv_type)
   {
      case IMV_CHAR:
         return sizeof(char);
      case IMV_UCHAR:
         return sizeof(unsigned char);
      case IMV_SHORT:
         return sizeof(short);          
      case IMV_USHORT:
         return sizeof(unsigned short);         
      case IMV_INT:
         return sizeof(int);
      case IMV_UINT:
         return sizeof(unsigned int);           
      case IMV_LONG:
         return sizeof(long);           
      case IMV_ULONG:
         return sizeof(unsigned long);          
      case IMV_LONGLONG:
         return sizeof(long long);       
      case IMV_ULONGLONG:
         return sizeof(unsigned long long);      
      case IMV_FLOAT:
         return sizeof(float);          
      case IMV_DOUBLE:
         return sizeof(double);         
      case IMV_LONGDOUBLE:
         return sizeof(long double);     
      case IMV_CMPLXFLOAT:
         return sizeof(std::complex<float> );     
      case IMV_CMPLXDOUBLE:
         return sizeof(std::complex<double> );    
      case IMV_CMPLXLONGDOUBLE:
         return sizeof(std::complex<long double> );
      default:
         return 0;
   }
   
   return 0;
}
