#ifndef __TM_TM__
#define __TM_TM__

#include <linux/time.h>

unsigned long get_seconds_since_last_read(void);
int get_absolute_time_since_last_read(struct tm *abs_time);

int init_sequence(void);
unsigned long get_next_in_sequence(void);
void destroy_sequence(void);

#endif // __TM_TM__
