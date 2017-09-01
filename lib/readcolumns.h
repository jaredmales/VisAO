
#include <iostream>
#include <fstream>

#include <stdarg.h>
#include <cstring>

#include <cstdlib>

#ifndef __readcol_h__
#define __readcol_h__

int readcolumns(const char * fname,  std::string delims, char comment, const char * format, ...);

#endif //__readcol_h__
