#include <linux/module.h>
#include <linux/timer.h>
#include <linux/ctype.h>
#include <linux/init.h>



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Maksim Holikiv  golikov.mo@gmail.com");
MODULE_DESCRIPTION("Modul for test system calls");
MODULE_VERSION("0.1");


#define MAX_VALUE_OF_FEBONACHY  20

struct timer_list workTimer;




static u32 febNum;
module_param( febNum, int, S_IRUGO );

static u32 counterToOff;
module_param( counterToOff, int, S_IRUGO );

static u32 timerPeriod;
module_param( timerPeriod, int, S_IRUGO );






static void  timerCallback(struct timer_list *workTimer);
       u32   Febonachy(u32 value);


static int __init initial_func(void) {
    
    if(febNum > MAX_VALUE_OF_FEBONACHY){
      printk("Number to calculate febonachy is  %d", febNum);
    }else{
      printk("Entred number more then %d, seted value is", MAX_VALUE_OF_FEBONACHY, 5);
    }
	
	
    printk("Period of timer is  %d", timerPeriod);
    printk("timer counter is  %d",   counterToOff);
   
      

    __init_timer(&workTimer, timerCallback, 0);
    mod_timer( &workTimer, jiffies + msecs_to_jiffies(timerPeriod));   	
    printk("timer was initialuzed. Time to wake up  %d \n", workTimer.expires);

return 0;
}


void  timerCallback(struct timer_list *workTimer){
    
    u32 FebonachyRes = Febonachy(febNum);
    printk("Febonachy is %d",FebonachyRes);
    
    if(counterToOff){
      counterToOff --;
      mod_timer( workTimer, jiffies + msecs_to_jiffies(timerPeriod) );
    }

 }


u32 Febonachy(u32 value){
    u32 result = 0;
	if(value < MAX_VALUE_OF_FEBONACHY){
      if(value > 1){
         result = Febonachy(value - 1) + Febonachy(value - 2);
      }else{
      	 result = 1;
      }   
	}else{
		result = MAX_VALUE_OF_FEBONACHY;
	}
	return result;
}


static void __exit unloading(void) {
    del_timer( &workTimer );   
    printk("timer modul unload sucsessfully\n");
}

module_init(initial_func);
module_exit(unloading);
