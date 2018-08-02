#include "config.h"
#include "tm.h"

#include <linux/jiffies.h>
#include <linux/ktime.h>
#include <linux/slab.h>
#include <linux/time64.h>
#include <linux/timekeeping.h>
#include <linux/timer.h>


unsigned long get_seconds_since_last_read(void)
{
	static int is_reqeusted_before = 0;
	static u64 last = 0;

	unsigned long res = 0;

	if (is_reqeusted_before) {
		res = (unsigned long)(get_jiffies_64() - last);
		res = res / HZ;
	}

	is_reqeusted_before = 1;
	last = get_jiffies_64();
	return res;
}


int get_absolute_time_since_last_read(struct tm *abs_time)
{
	static struct timespec64 tv;
	static int is_called = 0;
	int res = -EAGAIN;

	if (is_called) {
		time64_to_tm(tv.tv_sec, 0, abs_time);
		res = 0;
	}

	is_called = 1;
	getnstimeofday64(&tv);
	return res;
}


struct sequence_timer
{
	struct timer_list timer;
	unsigned long val;
};

static struct sequence_timer *seq_timer = NULL;


static void seq_timer_callback(unsigned long data)
{
	seq_timer->val += 5;

	if (mod_timer(&seq_timer->timer, jiffies + msecs_to_jiffies(1000)))
		printk(KERN_ERR MODULE_NAME ": failed to modify timer");
}


int init_sequence(void)
{
	int res = -ENOMEM;
	WARN_ON( seq_timer );

	do {
		seq_timer = kmalloc(sizeof(struct sequence_timer), GFP_KERNEL);

	if (!seq_timer)
		break;

	setup_timer(&seq_timer->timer, &seq_timer_callback, 0);
	seq_timer->val = 0;

	res = mod_timer(&seq_timer->timer, jiffies + msecs_to_jiffies(1000));
	} while(0);

	return res;
}


unsigned long get_next_in_sequence(void)
{
	return (seq_timer) ? seq_timer->val : 0;
}


void destroy_sequence(void)
{
	WARN_ON( !seq_timer );
	if (del_timer(&seq_timer->timer))
		printk(KERN_WARNING MODULE_NAME ": timer still in use\n");

	if (seq_timer) {
		kfree(seq_timer);
		seq_timer = NULL;
	}
}
