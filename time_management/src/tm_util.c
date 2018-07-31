#include "tm_util.h"

#include <asm-generic/errno-base.h>
#include <linux/time.h>


int val_to_str(unsigned long val, char *buf, size_t size)
{
	int res;

	res = snprintf(buf, size, "%lu", val);
	return (res < 1) ? -EIO : ++res;
}


int date_to_str(const struct tm *abs_time, char *buf, size_t size)
{
	int res;
	char *tz_sign = "+";

	if (sys_tz.tz_minuteswest < 0)
		tz_sign = "-";

	res = snprintf(buf, size, "%04ld-%02d-%02d %02d:%02d:%02d UTC %s%02d:%02d", 1900 + abs_time->tm_year
		, abs_time->tm_mon, abs_time->tm_mday, abs_time->tm_hour, abs_time->tm_min, abs_time->tm_sec, tz_sign
		, sys_tz.tz_minuteswest / 60, sys_tz.tz_minuteswest % 60);
	return (res < 1) ? -EIO : ++res;
}
