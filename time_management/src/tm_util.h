#ifndef __TM_TM_UTIL__
#define __TM_TM_UTIL__

#include "tm.h"

int val_to_str(unsigned long val, char *buf, size_t size);
int date_to_str(const struct tm *abs_time, char *buf, size_t size);

#endif // __TM_TM_UTIL__
