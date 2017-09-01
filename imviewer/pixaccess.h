



#ifndef __pixaccess_h__
#define __pixaccess_h__

#include <cstdlib>
#include <complex>

#define SCEXAO_CHAR 1
#define SCEXAO_INT 2
#define SCEXAO_FLOAT 3
#define SCEXAO_DOUBLE 4
#define SCEXAO_COMPLEX_FLOAT 5
#define SCEXAO_COMPLEX_DOUBLE 6
#define SCEXAO_USHORT 7
#define SCEXAO_LONG 8


#define IMV_CHAR             0
#define IMV_UCHAR            1
#define IMV_SHORT            2
#define IMV_USHORT           3
#define IMV_INT              4
#define IMV_UINT             5
#define IMV_LONG             6
#define IMV_ULONG            7
#define IMV_LONGLONG         8
#define IMV_ULONGLONG        9
#define IMV_FLOAT            10
#define IMV_DOUBLE           11
#define IMV_LONGDOUBLE       12
#define IMV_CMPLXFLOAT       13
#define IMV_CMPLXDOUBLE      14
#define IMV_CMPLXLONGDOUBLE  15

template<typename dataT>
float getPix(void *imdata, size_t idx)
{
   return (float) ((dataT*) imdata)[idx];
}

float (*getPixPointer(int imv_type))(void*, size_t);

size_t sizeof_imv_type(int imv_type);




#endif



