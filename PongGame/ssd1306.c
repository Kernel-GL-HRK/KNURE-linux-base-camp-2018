#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h> 

#include "images.h"   
#include "logic.h"                                                                     
#include "fonts.h"


#define DEVICE_NAME	"lcd_ssd1306"
#define CLASS_NAME	"oled_display" 
#define BUS_NAME	"i2c_1"

#define SSD1306_WIDTH	128
#define SSD1306_HEIGHT	64


static struct device *dev;

#define DEVICE_NAME "lcd1306"   /* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80              /* Max length of the message from the device */

static int Major;           /* Major number assigned to our device driver */
static int Device_Open = 0; /* Is device open?  
                             * Used to prevent multiple access to device */
static char msg[BUF_LEN];   /* The msg the device will give when asked */
static char *msg_Ptr;

extern int GLOBAL_VARIABLE;

static ssize_t paint_show(void);


/* 
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
    static int counter = 0;

    if (Device_Open)
        return -EBUSY;

    Device_Open++;
    sprintf(msg, "I already told you %d times Hello world!\n", counter++);
    msg_Ptr = msg;
    try_module_get(THIS_MODULE);
    paint_show();
    return 0;
}

/* 
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
    Device_Open--;      /* We're now ready for our next caller */

    /* 
     * Decrement the usage count, or else once you opened the file, you'll
     * never get get rid of the module. 
     */
    module_put(THIS_MODULE);

    return 0;
}

/* 
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,   /* see include/linux/fs.h   */
               char *buffer,    /* buffer to fill with data */
               size_t length,   /* length of the buffer     */
               loff_t * offset)
{
    /*
     * Number of bytes actually written to the buffer 
     */
    int bytes_read = 0;

    /*
     * If we're at the end of the message, 
     * return 0 signifying end of file 
     */
    if (*msg_Ptr == 0)
        return 0;

    /* 
     * Actually put the data into the buffer 
     */
    while (length && *msg_Ptr) {

        /* 
         * The buffer is in the user data segment, not the kernel 
         * segment so "*" assignment won't work.  We have to use 
         * put_user which copies data from the kernel data segment to
         * the user data segment. 
         */
        put_user(*(msg_Ptr++), buffer++);

        length--;
        bytes_read++;
    }

    /* 
     * Most read functions return the number of bytes put into the buffer
     */
    return bytes_read;
}

/*  
 * Called when a process writes to dev file: echo "hi" > /dev/hello 
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
    printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
    return -EINVAL;
}


static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};


struct ssd1306_data {
    struct i2c_client *client;
	struct class *sys_class;
    int status;

	int value;
};
static struct ssd1306_data *lcd;


/* SSD1306 data buffer */
static u8 ssd1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];


/**
* @brief SSD1306 color enumeration
*/
typedef enum {
        SSD1306_COLOR_BLACK = 0x00,   /*!< Black color, no pixel */
        SSD1306_COLOR_WHITE = 0x01    /*!< Pixel is set. Color depends on LCD */
} ssd1306_COLOR_t;


typedef struct{
	u16 	X;
	u16 	Y;
}_Point;


/* Init sequence taken from the Adafruit SSD1306 Arduino library */
static void ssd1306_init_lcd(struct i2c_client *drv_client) {

    char m;
    char i;

    dev_info(dev, "ssd1306: Device init \n");
    	/* Init LCD */    
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xAE); //display off
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x20); //Set Memory Addressing Mode
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xB0); //Set Page Start Address for Page Addressing Mode,0-7
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xC8); //Set COM Output Scan Direction
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x00); //---set low column address
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x10); //---set high column address
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x40); //--set start line address
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x81); //--set contrast control register
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x0A);
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xA1); //--set segment re-map 0 to 127
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xA6); //--set normal display
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xA8); //--set multiplex ratio(1 to 64)
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x3F); //
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xD3); //-set display offset
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x00); //-not offset

    i2c_smbus_write_byte_data(drv_client, 0x00, 0xD5); //--set display clock divide ratio/oscillator frequency
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xa0); //--set divide ratio
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xD9); //--set pre-charge period
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x22); //

    i2c_smbus_write_byte_data(drv_client, 0x00, 0xDA); //--set com pins hardware configuration
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x12);
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xDB); //--set vcomh
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x20); //0x20,0.77xVcc
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x8D); //--set DC-DC enable
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x14); //
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xAF); //--turn on SSD1306 panel
    
    for (m = 0; m < 8; m++) {
        i2c_smbus_write_byte_data(drv_client, 0x00, 0xB0 + m);
        i2c_smbus_write_byte_data(drv_client, 0x00, 0x00);
        i2c_smbus_write_byte_data(drv_client, 0x00, 0x10);
        // Write multi data 
        /*
        for (i = 0; i < SSD1306_WIDTH; i++) {
            i2c_smbus_write_byte_data(drv_client, 0x40, 0xaa);
        }
        */
    }   
}

int ssd1306_UpdateScreen(struct ssd1306_data *drv_data) {

    struct i2c_client *drv_client;
    char m;
    char i;

    drv_client = drv_data->client;

    for (m = 0; m < 8; m++) {
        i2c_smbus_write_byte_data(drv_client, 0x00, 0xB0 + m);
        i2c_smbus_write_byte_data(drv_client, 0x00, 0x00);
        i2c_smbus_write_byte_data(drv_client, 0x00, 0x10);
        /* Write multi data */
        for (i = 0; i < SSD1306_WIDTH; i++) {
            i2c_smbus_write_byte_data(drv_client, 0x40, ssd1306_Buffer[SSD1306_WIDTH*m +i]);
        }   
    }

    return 0;
}


int ssd1306_DrawPixel(struct ssd1306_data *drv_data, u16 x, u16 y, ssd1306_COLOR_t color) {

    if ( x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT ) {
        return -1;
    }

    /* Set color */
    if (color == SSD1306_COLOR_WHITE) {
        ssd1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    }
    else {
        ssd1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }

    return 0;
}


void Graphic_setPoint(const u16 X, const u16 Y)
{
	ssd1306_DrawPixel(lcd, X, Y, SSD1306_COLOR_WHITE);
}


void Graphic_drawLine(_Point p1, _Point p2){
	int dx, dy, inx, iny, e;
	u16 x1 = p1.X, x2 = p2.X;
	u16 y1 = p1.Y, y2 = p2.Y;

    //u16 Color = Graphic_GetForeground();

    dx = x2 - x1;
    dy = y2 - y1;
    inx = dx > 0 ? 1 : -1;
    iny = dy > 0 ? 1 : -1;

//	dx = (u16)abs(dx);
//    dy = (u16)abs(dy);
    dx = (dx > 0) ? dx : -dx;
    dy = (dy > 0) ? dy : -dy;


    if(dx >= dy) {
        dy <<= 1;
        e = dy - dx;
        dx <<= 1;
        while (x1 != x2){
        	Graphic_setPoint(x1, y1);//, Color);
			if(e >= 0){
				y1 += iny;
				e-= dx;
			}
			e += dy;
			x1 += inx;
		}
	}
    else{
		dx <<= 1;
		e = dx - dy;
		dy <<= 1;
		while (y1 != y2){
			Graphic_setPoint(x1, y1);//, Color);
			if(e >= 0){
				x1 += inx;
				e -= dy;
			}
			e += dx;
			y1 += iny;
		}
	}
    Graphic_setPoint(x1, y1);//, Color);
}
// ---------------------------------------------------------------------------


void Graphic_drawLine_(u16 x1, u16 y1, u16 x2, u16 y2){
	_Point p1 = {0}, p2 = {0};
	p1.X = x1;
	p1.Y = y1;
	p2.X = x2;
	p2.Y = y2;

	Graphic_drawLine(p1, p2);
}
// ---------------------------------------------------------------------------

void drawFastVLine(int16_t x, int16_t y,
        int16_t h) {
    
   Graphic_drawLine_(x, y, x, y+h-1);
    
}

void writeFastVLine(int16_t x, int16_t y,
        int16_t h) {
    // Overwrite in subclasses if startWrite is defined!
    // Can be just writeLine(x, y, x, y+h-1, color);
    // or writeFillRect(x, y, 1, h, color);
    drawFastVLine(x, y, h);
}


/**************************************************************************/
/*!
   @brief    Quarter-circle drawer with fill, used to do circles and roundrects
    @param    x0   Center-point x coordinate
    @param    y0   Center-point y coordinate
    @param    r   Radius of circle
    @param    cornername  Mask bit #1 or bit #2 to indicate which quarters of the circle we're doing
    @param    delta  Offset from center-point, used for round-rects
    @param    color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void fillCircleHelper(int16_t x0, int16_t y0, int16_t r,uint8_t cornername, int16_t delta) {

    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;

        if (cornername & 0x1) {
            writeFastVLine(x0+x, y0-y, 2*y+1+delta);
            writeFastVLine(x0+y, y0-x, 2*x+1+delta);
        }
        if (cornername & 0x2) {
            writeFastVLine(x0-x, y0-y, 2*y+1+delta);
            writeFastVLine(x0-y, y0-x, 2*x+1+delta);
        }
    }
}

/**************************************************************************/
void fillCircle(int16_t x0, int16_t y0, int16_t r) {
        
    writeFastVLine(x0, y0-r, 2*r+1);
    fillCircleHelper(x0, y0, r, 3, 0);
    
}











//------------------------------------------



void drawCircle(int16_t x0, int16_t y0, int16_t radius) {
  int16_t x = 0, y = radius;
    int16_t dp = 1 - radius;
    do {
        if (dp < 0)
            dp = dp + 2 * (++x) + 3;
        else
            dp = dp + 2 * (++x) - 2 * (--y) + 5;

        Graphic_setPoint(x0 + x, y0 + y);     //For the 8 octants
        Graphic_setPoint(x0 - x, y0 + y);
        Graphic_setPoint(x0 + x, y0 - y);
        Graphic_setPoint(x0 - x, y0 - y);
        Graphic_setPoint(x0 + y, y0 + x);
        Graphic_setPoint(x0 - y, y0 + x);
        Graphic_setPoint(x0 + y, y0 - x);
        Graphic_setPoint(x0 - y, y0 - x);

    } while (x < y);

  Graphic_setPoint(x0 + radius, y0);
  Graphic_setPoint(x0, y0 + radius);
  Graphic_setPoint(x0 - radius, y0);
  Graphic_setPoint(x0, y0 - radius);
}







void ssd1306_clear(u8 color){

	memset(ssd1306_Buffer, color, sizeof(ssd1306_Buffer));

	//ssd1306_UpdateScreen(lcd);
}

static ssize_t clear_show(void)
{
	//int ret;
	ssize_t i = 0;

	dev_info(dev, "%s\n", __FUNCTION__);
	
	ssd1306_clear(0);
    ssd1306_UpdateScreen(lcd);

	return i;
}


static ssize_t paint_show(void)
{
	ssize_t i = 0;
	//_Point center = {64, 32};
	dev_info(dev, "%s\n", __FUNCTION__);

	ssd1306_clear(0);


	Graphic_drawLine_(0, 32, 80, 32);




	ssd1306_UpdateScreen(lcd);

	return i;
}


//---------------------------------------

_Point position ={0};

//Bitmap









 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))

void drawBitmap(int16_t x, int16_t y,
  const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color) {

    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;


    int16_t j,i;

    for(j=0; j<h; j++, y++) {
        for(i=0; i<w; i++) {
            if(i & 7) byte <<= 1;
            else      byte   = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
            if(byte & 0x80) Graphic_setPoint(x+i, y);
        }
    }

}

//
//  Draw 1 char to the screen buffer
//  ch      => char om weg te schrijven
//  Font    => Font waarmee we gaan schrijven
//  color   => Black or White
//
char ssd1306_WriteChar(char ch, FontDef Font, ssd1306_COLOR_t color)
{
    uint32_t i, b, j;
    

   
    // Use the font to write
    for (i = 0; i < Font.FontHeight; i++)
    {
        b = Font.data[(ch - 32) * Font.FontHeight + i];
        for (j = 0; j < Font.FontWidth; j++)
        {
            if ((b << j) & 0x8000) 
            {
                ssd1306_DrawPixel(lcd,position.X+ j, (position.Y + i), (ssd1306_COLOR_t ) color);
            } 
            else 
            {
                ssd1306_DrawPixel(lcd,position.X+ j, (position.Y + i), (ssd1306_COLOR_t )!color);
            }
        }
    }
    
    // The current space is now taken
  // position.Y += Font.FontWidth+2;
    position.X += Font.FontWidth+2;

    // Return written char for validation
    return ch;
}

//
//  Write full string to screenbuffer
//
char ssd1306_WriteString(char* str, FontDef Font, ssd1306_COLOR_t  color)
{
    // Write until null-byte
    while (*str) 
    {
        if (ssd1306_WriteChar(*str, Font, color) != *str)
        {
            // Char could not be written
            return *str;
        }
        
        // Next char
        str++;
    }
    
    // Everything ok
    return *str;
}

//
//  Position the cursor
//
void ssd1306_SetCursor(uint8_t x, uint8_t y) 
{
    position.X = x;
    position.Y = y;
}

//---------------------------------------



static struct task_struct *task;
static int data = 0x55;

int thread_function(void *data){

    unsigned char line=0;
    unsigned char lineSize=25;
    unsigned char flagLeftRight=1;

    int change_x=10;
    short snake_size=25;
    int change_y=10;
    unsigned char flag=1;

    unsigned char ball_x = 34, ball_y = 32;
    unsigned char ball_dir_x = 3, ball_dir_y = 3;

    unsigned char new_x;
    unsigned char new_y;

    static unsigned int counter=0;

    state_machine state=PRINT_LOGO;

    while(!kthread_should_stop()){

        switch (state){

            case PRINT_LOGO:
                            ssd1306_clear(0);
                            drawBitmap(32, 0,  GL, 64, 64, 1); //Print the GLlogo;
                            ssd1306_UpdateScreen(lcd);  
                            msleep(2000);
                            state = PRINT_LOGO2;
                            schedule();

                break;
            case PRINT_LOGO2:
                            ssd1306_clear(0);
                            drawBitmap(5, 15,  global, 120, 34, 1); //Print the GLlogo;
                            ssd1306_UpdateScreen(lcd);  
                            msleep(2000);
                            state = GAME;
                            schedule();

                break;


            case GAME:

                            new_x = ball_x + ball_dir_x;
                            new_y = ball_y + ball_dir_y;
                            

                            // Check if we hit 
                            if(new_y >= 60 && new_x >= line && new_x <= line+lineSize) {
                            ball_dir_x = -ball_dir_x;
                            new_x += ball_dir_x + ball_dir_x;
                            counter+=1;
                            }    

                            // Check if we hit the vertical walls
                            if(new_x <= 5 || new_x >= 122) {
                                ball_dir_x = -ball_dir_x;
                                new_x += ball_dir_x + ball_dir_x;
                            }

                            // Check if we hit the horizontal walls.
                            if(new_y <= 20 || new_y >= 62) {
                                ball_dir_y = -ball_dir_y;
                                new_y += ball_dir_y + ball_dir_y;
                            }
                            

                            ssd1306_clear(0);


                            fillCircle(new_x,new_y,5);
                            
                           // if (flagLeftRight)line+=6;
                            //else line-=6;

                            switch (flagLeftRight){
                                    case 1:
                                        line+=6;
                                        break;
                                     case 2:
                                        line-=6;
                                        break;
                             break;                  
                             }   

                            if (line>=103)line-=6;
                            else if(line<=0)line+=6;

                            if ((GLOBAL_VARIABLE>1500)&&(GLOBAL_VARIABLE<2500))flagLeftRight=0;
                            else if (GLOBAL_VARIABLE<1500)flagLeftRight=1;
                            else if (GLOBAL_VARIABLE>2500)flagLeftRight=2;

                            Graphic_drawLine_(line, 62, line+lineSize, 62);// 
                            Graphic_drawLine_(line, 63, line+lineSize, 63);//
                            

                            ssd1306_SetCursor(10,2);
                            ssd1306_WriteString("Counter:",Font_7x10,SSD1306_COLOR_WHITE);

                            ssd1306_SetCursor(80,2);
                            
                            char temp_string[4];
                            sprintf(temp_string,"%zu",counter);

                            ssd1306_WriteString(temp_string,Font_7x10,SSD1306_COLOR_WHITE);
                            ssd1306_UpdateScreen(lcd); 
                            msleep(30);
                            schedule();

                            ball_x = new_x;
                            ball_y = new_y;

                break;    



            break;
        }
        

    }

    return 0;
}



static int ssd1306_probe(struct i2c_client *drv_client,
			const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter;

	dev = &drv_client->dev;

	dev_info(dev, "init I2C driver\n");


    lcd = devm_kzalloc(&drv_client->dev, sizeof(struct ssd1306_data),
                        GFP_KERNEL);
    if (!lcd)
        return -ENOMEM;

    lcd->client = drv_client;
    lcd->status = 0xABCD;
    lcd->value 	= 10;

    i2c_set_clientdata(drv_client, lcd);

    adapter = drv_client->adapter;

    if (!adapter)
    {
        dev_err(dev, "adapter indentification error\n");
        return -ENODEV;
    }

    dev_info(dev, "I2C client address %d \n", drv_client->addr);

    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
            dev_err(dev, "operation not supported\n");
            return -ENODEV;
    }



    ssd1306_init_lcd(drv_client);
    //ssd1306_UpdateScreen(lcd);

    dev_info(dev, "ssd1306 driver successfully loaded Mordik\n");

	return 0;
}



static int ssd1306_remove(struct i2c_client *client)
{
	dev_info(dev, "Goodbye, world!\n");
	return 0;
}



static const struct of_device_id ssd1306_match[] = {
	{ .compatible = "DAndy,lcd_ssd1306", },
	{ },
};
MODULE_DEVICE_TABLE(of, ssd1306_match);

static const struct i2c_device_id ssd1306_id[] = {
	{ "lcd_ssd1306", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ssd1306_id);


static struct i2c_driver ssd1306_driver = {
	.driver = {
		.name	= DEVICE_NAME,
		.of_match_table = ssd1306_match,
	},
	.probe		= ssd1306_probe,
	.remove 	= ssd1306_remove,
	.id_table	= ssd1306_id,
};


static int __init mod_init(void){
    i2c_add_driver(&ssd1306_driver);
    printk("driver added!\n");
       Major = register_chrdev(0, DEVICE_NAME, &fops);

    if (Major < 0) {
      printk(KERN_ALERT "Registering char device failed with %d\n", Major);
      return Major;
    }

    printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
    printk(KERN_INFO "the driver, create a dev file with\n");
    printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
    printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
    printk(KERN_INFO "the device file.\n");
    printk(KERN_INFO "Remove the device file and module when done.\n");

        task = kthread_run(&thread_function,(void *)data,"Blablathread");
    if(task)
        printk(KERN_INFO "Thread created successfully\n");
    else
        printk(KERN_INFO "Thread creation failed\n");

    return 0;
}

static void __exit mod_exit(void)
{
    printk("remove driver\n");
    i2c_del_driver(&ssd1306_driver);
    printk("driver removed\n");
        kthread_stop(task);
    unregister_chrdev(Major, DEVICE_NAME);

}

module_init( mod_init );
module_exit( mod_exit );

MODULE_AUTHOR("Devyatov Andrey <andrii.deviatov@globallogic.com>");
MODULE_DESCRIPTION("ssd1306 I2C");
MODULE_LICENSE("GPL");
