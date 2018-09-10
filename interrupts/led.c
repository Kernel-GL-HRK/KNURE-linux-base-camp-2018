#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm-generic/errno.h>

#define DEVICE_NAME	"phenn_led"
#define CLASS_NAME "led"

static unsigned int gpio_led_num = 15;
static unsigned int gpio_but_num = 355;
static unsigned int irq_num;
static short led_on = 0;

//static struct task_struct *blink_thread;

static int gpio_led_init( void );
static int gpio_but_init( void );

/*static int blink_func(void* data) {
	while (!kthread_should_stop()) {
		led_on = !led_on;
		gpio_set_value(gpio_led_num, led_on);
		msleep(500);
	}
	return 0;
}*/

static irq_handler_t but_handler(int irq, void* dev_id) {
	led_on = !led_on;
	gpio_set_value(gpio_led_num, led_on);
	return IRQ_HANDLED;
}

static int gpio_led_init( void ) {
	int err = 0;

	if (!gpio_is_valid(gpio_led_num)) {
		printk(KERN_ALERT "Led: bad device");
		goto fail;
	}

	err = gpio_request(gpio_led_num, "sysfs");
	if (err < 0) {
		printk(KERN_ALERT "Led: bad gpio_request");
		goto fail;
	}

	err = gpio_direction_output(gpio_led_num, led_on);
	if (err < 0) {
		printk(KERN_ALERT "Led: cannot set gpio to output");
		goto fail;
	}

	err = gpio_export(gpio_led_num, led_on);
	if (err < 0) {
		printk(KERN_ALERT "Led: cannot export gpio number");
		goto fail;	
	}
	printk(KERN_INFO "Led: device valid");

	return 0;
fail:
	return err;
}

static int gpio_but_init( void ) {
	int err = 0;

	if (!gpio_is_valid(gpio_but_num)) {
		printk(KERN_ALERT "Button: bad device");
		return -ENODEV;
	}

	err = gpio_request(gpio_but_num, "sysfs");
	if (err < 0) {
		printk(KERN_ALERT "Button: bad gpio_request");
		goto fail;
	}

	err = gpio_direction_input(gpio_but_num);
	if (err < 0) {
		printk(KERN_ALERT "Button: cannot set gpio to output");
		goto fail;
	}

	err = gpio_export(gpio_but_num, led_on);
	if (err < 0) {
		printk(KERN_ALERT "Button: cannot export gpio number");
		goto fail;
	}
	
	printk(KERN_INFO "Button: device valid");

	return 0;
fail:
	return err;
}

static int __init led_init( void ) {
	int err = 0;

	printk(KERN_INFO "Init module...");
	
	led_on = 1;

	err = gpio_led_init();
	if (err < 0) {
		printk(KERN_ALERT "Led: init failed. Stop.");
		return err;
	}

	err = gpio_but_init();
	if (err < 0) {
		printk(KERN_ALERT "Button: init failed. Stop.");
		return err;
	}
	
	irq_num = gpio_to_irq(gpio_but_num);
	printk (KERN_INFO "Button: irq number is %d", irq_num);

	err = request_irq(irq_num,
					  (irq_handler_t)but_handler,
					  IRQF_TRIGGER_RISING,
					  "button_handler", 
					  NULL);

	if (err != 0) {
		printk(KERN_ALERT "Button: cannot request irq, errno=%d", err);
		return err;
	}
	/*blink_thread = kthread_run(blink_func, NULL, "led_blink");
	if (!blink_thread) {
		printk(KERN_ALERT "Led: thread wasn't created");
		return -EINVAL;
	}
	printk(KERN_INFO "Led: thread created");
	*/
	printk(KERN_INFO "Led: module initialized successfully");
	return err;
}

static void __exit led_exit( void ) {
	/*int err = 0;

	if (blink_thread) {
		err = kthread_stop(blink_thread);
		blink_thread = NULL;
		if (err < 0) {
			printk(KERN_INFO "Led: thread wasn't created");
		}
	}*/

	gpio_unexport(gpio_but_num);
	gpio_free(gpio_but_num);
	free_irq(irq_num, NULL);
	gpio_set_value(gpio_led_num, 0);
	gpio_unexport(gpio_led_num);
	gpio_free(gpio_led_num);
	
	printk(KERN_INFO "Module removed");
}

module_init(led_init);
module_exit(led_exit);

MODULE_AUTHOR("Ponedelnikov Hennadii");
MODULE_DESCRIPTION("gpio_led");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
