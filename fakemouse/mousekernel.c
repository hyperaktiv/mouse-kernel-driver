#ifndef __KERNEL__
#  define __KERNEL__
#endif
#ifndef MODULE
#  define MODULE
#endif

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

/* usb stuff */
#include <linux/input.h>
#include <linux/usb.h>

/* miscdevice stuff */
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

/*
 * Version Information
 */
#define DRIVER_VERSION "v0.1"
#define DRIVER_AUTHOR "Github from Kush"
#define DRIVER_DESC "Mouse driver"
#define DRIVER_LICENSE "GPL"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENSE);

#define DEVICE_NAME "mousekernel"
#define MAX_SCREEN 100
#define MIN_SCREEN 0

int Major;

/*
 * We need a local data structure, as it must be allocated for each new
 * mouse device plugged in the USB bus
 */

struct mousek_device {
    signed char data[4];     /* use a 4-byte protocol */
    struct urb urb;          /* USB Request block, to get USB data*/
    struct input_dev *idev;   /* input device, to push out input  data */
    int x, y;                /* keep track of the position of this device */
};

static struct mousek_device *mouse;

/*
 * Handler for data sent in by the device. The function is called by
 * the USB kernel subsystem whenever the device spits out new data
 
static void mousek_irq(struct urb *urb)
{ //useless right now
    struct mousek_device *mouse = urb->context;
    signed char *data = mouse->data;
    struct input_dev *idev = &mouse->idev;
	
    if (urb->status != USB_ST_NOERROR) return;
    // ignore data[0] which reports mouse buttons 
    mouse->x += data[1];
    mouse->y += data[2];
    printk(KERN_DEBUG "mousek irq, %5i %5i\n", mouse->x, mouse->y);
    input_report_rel(mouse->idev, REL_X,     mouse->x);
	input_report_rel(mouse->idev, REL_Y,     mouse->y);
	input_report_rel(mouse->idev, REL_WHEEL, data[3]);
	input_sync(idev);
}
*/

/*
static irqreturn_t mousek_interrupt(int irq, void *dummy)
{
	input_report_key(mouse->idev, BTN_0, inb(BUTTON_PORT) & 1);
	input_sync(mouse->idev);
	return IRQ_HANDLED;
}
*/


/*
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
int mousek_open(struct inode *inode, struct file *filp)
{
    

    //filp->private_data = mouse;

    /* announce yourself */
    printk(KERN_INFO "mousekernel: faking an USB mouse via the misc device\n");
    
	try_module_get(THIS_MODULE);
	
	return 0; /* Ok */
}    

/* close releases the device, like mousek_disconnect */
int mousek_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "mousekernel: closing misc device\n");
    
	/*
	* Decrement the usage count, or else once you opened the file, you'll
    * never get rid of the module.
    */
    module_put(THIS_MODULE);
	
	return 0;
}

/* poll reports the device as writeable */
ssize_t mousek_read(struct file *filp,   /* see include/linux/fs.h   */
                           char *buffer,        /* buffer to fill with data */
                           size_t length,       /* length of the buffer     */
                           loff_t * offset)
{
    printk(KERN_INFO "mousekernel: Restricted. That is okay but do not repeat this mistake.\n");
    return 0;
}

/* write accepts data and converts it to mouse movement
 * Called when a process writes to dev file: echo "hi" > /dev/hello
 */
ssize_t mousek_write(struct file *filp, const char *buf, size_t count,
		    loff_t *offp)
{
    //struct mousek_device *mouse = filp->private_data;
    static char localbuf[16];
    //struct urb urb;
    int i, command = -1;

    /* accept 16 bytes at a time, at most */
    if (count >16) count=16;
    copy_from_user(localbuf, buf, count);

	struct input_dev *dev = mouse->idev;
	
	//first character will tell what command to do
	//i : instruction sequence like ududdr
	//x : absolute value in X Axis
	//y : absolute value in Y Axis
	//l : left click single, ll : double left click
	//r : right click

	switch(localbuf[0]){
	    case 'i': case 'I':{
	    	command = 0;
			break;
		}
		case 'x': case 'X':{
			command = 1;
			break;
		}
		case 'y': case 'Y':{
			command = 2;
			break;
		}
		case 'l': case 'L':{
			//left click press down
			input_report_key(dev, BTN_LEFT, 1);
			//left click up
			input_report_key(dev, BTN_LEFT, 0);
			
			input_sync(dev);

			if(localbuf[1] == 'l')
			{
				//left click press down
				input_report_key(dev, BTN_LEFT, 1);
				//left click up
				input_report_key(dev, BTN_LEFT, 0);
			}
			
			break;
		}
		case 'r': case 'R':{
			//right click press down
			input_report_key(dev, BTN_RIGHT, 1);
			//right click up
			input_report_key(dev, BTN_RIGHT, 0);
			break;
		}

	}
	
	if(command == 0){
		/* scan written sequence */
    	for (i=2; i<count; i++) {
			mouse->data[1] = mouse->data[2] = mouse->data[3] = 0;
			
			switch (localbuf[i]) {
			    case 'u': case 'U': /* up */
					mouse->data[2] = -10;
					break;
			    case 'd': case 'D': /* down */
					mouse->data[2] = 10;
					break;
			    case 'l': case 'L': /* left */
					mouse->data[1] = -10;
					break;
			    case 'r': case 'R': /* right */
					mouse->data[1] = 10;
					break;
					
				case 'q': /* left click down */
					mouse->data[3] = 1;
					break;
				case 'Q': /* left click up */
					mouse->data[3] = 2;
					break;
				case 'w': /* right click down */
					mouse->data[3] = 3;
					break;
				case 'W': /* right click up */
					mouse->data[3] = 4;
					break;
			    default:
			    	i = count;
				continue;
			}

			
			if(mouse->data[1] != 0){
				input_report_rel(dev, REL_X, mouse->data[1]);
				
				input_sync(dev);

			}
			if(mouse->data[2] != 0){
				input_report_rel(dev, REL_Y, mouse->data[2]);
				
				input_sync(dev);

			}

			//handle queue clicks
			if(mouse->data[3] != 0){
				if(mouse->data[3] == 1){
					input_report_key(dev, BTN_LEFT, 1);					
				}else if(mouse->data[3] == 2){
					input_report_key(dev, BTN_LEFT, 0);
				}else if(mouse->data[3] == 3){
					input_report_key(dev, BTN_RIGHT, 1);
				}else if(mouse->data[3] == 4){
					input_report_key(dev, BTN_RIGHT, 0);
				}				
				input_sync(dev);
			} 
			
			
			printk(KERN_ALERT "mousekernel: Control actions of %s received %d %d",localbuf, mouse->data[1], mouse->data[2]);

    	}//for
		
	}
	if(command == 1 && count >= 2){
		//X
		int val = 0;
		if(localbuf[2] == '-'){ 
			//negative
			val = localbuf[3] - '0'; //converting to int	
			for (i=4; i < count; i++) {
				if(localbuf[i] <= '9' && localbuf[i] >= '0'){
					val *= 10;
					val += localbuf[i] - '0'; 					
				}
			}			
			val *= -1;			
		}else{
			//if not negative
			val = localbuf[2] - '0'; //converting to int	
			for (i=3; i < count; i++) {
				if(localbuf[i] <= '9' && localbuf[i] >= '0'){
					val *= 10;
					val += localbuf[i] - '0'; 					
				}
			}
		}
		printk(KERN_ALERT "Moving with value %d in X\n", val);
		
		input_report_rel(dev, REL_X, val);
		//input_report_abs(dev, ABS_X, val);
	}
	if(command == 2 && count >= 2){
		//Y
		int val = 0;
		if(localbuf[2] == '-'){ 
			//negative
			val = localbuf[3] - '0'; //converting to int	
			for (i=4; i < count; i++) {
				if(localbuf[i] <= '9' && localbuf[i] >= '0'){
					val *= 10;
					val += localbuf[i] - '0'; 					
				}
			}			
			val *= -1;			
		}else{
			//if not negative
			val = localbuf[2] - '0'; //converting to int	
			for (i=3; i < count; i++) {
				if(localbuf[i] <= '9' && localbuf[i] >= '0'){
					val *= 10;
					val += localbuf[i] - '0'; 					
				}
			}
		}
		
		input_report_rel(dev, REL_Y, val);
		//input_report_abs(dev, ABS_Y, val);
	}

	input_sync(dev);
	
    return count;
}


struct file_operations mousek_fops = {
	write:    mousek_write,
	read:     mousek_read,
	open:     mousek_open,
	release:  mousek_release,
};

/*
 * Functions called at module load and unload time: only register and
 * unregister the USB callbacks and the misc entry point
 */
int init_module(void)
{
    int retval;
	
	Major = register_chrdev(0, DEVICE_NAME, &mousek_fops);
	if (Major < 0) {
	  printk(KERN_ALERT "Registering char device failed with %d\n", Major);
	  return Major;
	}
	
	struct input_dev *input_dev;

    /* allocate and zero a new data structure for the new device */
    mouse = kmalloc(sizeof(struct mousek_device), GFP_KERNEL);
    if (!mouse) return -ENOMEM; /* failure */
    memset(mouse, 0, sizeof(*mouse));

	input_dev = input_allocate_device();
	if (!input_dev) {
		printk(KERN_ERR "mousekernel.c: Not enough memory\n");
		retval = -ENOMEM;
		//goto err_free_irq;
	}
	//updating struct
	mouse->idev = input_dev;
    
	
	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
	

	input_dev->keybit[BIT_WORD(BTN_MOUSE)] = BIT_MASK(BTN_LEFT) | BIT_MASK(BTN_RIGHT) | BIT_MASK(BTN_MIDDLE);
	input_dev->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y) | BIT_MASK(REL_WHEEL);
	//input_dev->absbit[0] = BIT_MASK(ABS_X) | BIT_MASK(ABS_Y);
	
	//input_set_abs_params(input_dev, ABS_X, MIN_SCREEN, MAX_SCREEN, 0, 0);
	//input_set_abs_params(input_dev, ABS_Y, MIN_SCREEN, MAX_SCREEN, 0, 0);

	
	input_dev->name = DEVICE_NAME;	
	input_set_drvdata(input_dev, mouse);
	
	retval = input_register_device(input_dev);
	if (retval) {
		printk(KERN_ERR "mousekernel: Failed to register device\n");
		goto err_free_dev;
	}

	
	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a dev file with\n");   
    printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");
	
	
return 0;

err_free_dev:
	input_free_device(mouse->idev);
	kfree(mouse);
//err_free_irq:
//	free_irq(BUTTON_IRQ, button_interrupt);
return retval;
}

void cleanup_module(void)
{
    /*
    * Unregister the device
    */
	if(!mouse) return;
	
	input_unregister_device(mouse->idev);
	kfree(mouse);	
	unregister_chrdev(Major, DEVICE_NAME);
    
	printk(KERN_ALERT "Uninstalled. Delete device from dev.");
}
