// Epoch conversion header file
#include "definitions.h"

#ifndef _EPOCH_CONV_
// Epoch time processing definitions

//        const unsigned char month_epoch = 1, day_epoch = 1, hour_poch = 0, minute_poch = 0;
 //       const int year_epoch = 1970;
  //      const double sec_ms_epoch = 0;
  //      unsigned char leap_years =0;
  //      const unsigned long month_s[] = {2678400, 5097600, 7776000,10368000,13046400,15638400,18316800,20995200,23587200,26265600,28857600,31536000}; // seconds enumeration

// End of epoch time processing definitions

uint64_t date_to_epoch(char* line);
uint64_t convert_date_to_epoch(char* line);
#define _EPOCH_CONV_
#endif
