
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/signal.h>

#include "driver-gamepad.h"
#include "gpio.h"

// Module parameters
#define DRIVER_NAME "gamepad"

// Function forward declarations
static int gp_probe(struct platform_device *p_dev_ptr);
static int gp_remove(struct platform_device *dev);
static int gp_open(struct inode *inode, struct file *file_pointer);
static int gp_release(struct inode *inode, struct file *file_pointer);
static long gp_ioctl(struct file *file_pointer, unsigned int cmd,
		     unsigned long arg);
static int gp_fasync(int fd, struct file *filp, int on);

// Module variables
static struct class *gp_cl;
static struct cdev *gp_cdev_ptr;
static dev_t gp_dev_number;
static struct fasync_struct *gp_async_queue;
static int unsigned gp_button_state;
static struct file_operations gp_fops = {
	.owner = THIS_MODULE,
	.open = gp_open,
	.release = gp_release,
	.unlocked_ioctl = gp_ioctl,
	.fasync = gp_fasync,
};

static const struct of_device_id gp_of_match[] = {
	{.compatible = "tdt4258",},
	{},
};

static struct platform_driver gp_driver = {
	.probe = gp_probe,
	.remove = gp_remove,
	.driver = {
		   .name = "gp",
		   .owner = THIS_MODULE,
		   .of_match_table = gp_of_match,
		   },
};

// Function definitions

static void setup_gpio(volatile void *gpio_base)
{
	int long unsigned current_value;
	int long unsigned new_value;

	// Enable buttons
	// Set GPIO PC to input
	new_value = 0x33333333;
	iowrite32(new_value, gpio_base + GPIO_PC_MODEL);

	// Enable internal pullup
	new_value = 0xFF;
	iowrite32(new_value, gpio_base + GPIO_PC_DOUT);

	// Enable interrupts for GPIO C when its state changes
	new_value = 0x22222222;
	iowrite32(new_value, gpio_base + GPIO_EXTIPSELL);

	// Set up the GPIO to interrupt when a bit changes from 1 to 0 (button pressed)
	new_value = 0xFF;
	iowrite32(new_value, gpio_base + GPIO_EXTIFALL);

	// Set up interrupt generation
	current_value = ioread32(gpio_base + GPIO_IEN);
	new_value = current_value | 0xFF;
	iowrite32(new_value, gpio_base + GPIO_IEN);

	// Clear interrupt flags to avoid interrupt on startup
	current_value = ioread32(gpio_base + GPIO_IFC);
	new_value = current_value | ioread32(gpio_base + GPIO_IF);
	iowrite32(new_value, gpio_base + GPIO_IFC);
}

static irqreturn_t isr_store_button_state(int irq, void *dev)
{
	int long unsigned current_value;
	int long unsigned new_value;

	volatile void *gpio_base =
	    (volatile void *)platform_get_resource(dev, IORESOURCE_MEM,
						   0)->start;

	// Clear the interrupt to avoid repeating interrupts
	current_value = ioread32(gpio_base + GPIO_IFC);
	new_value = current_value | ioread32(gpio_base + GPIO_IF);
	iowrite32(new_value, gpio_base + GPIO_IFC);

	// Store the button state
	gp_button_state = ioread32(gpio_base + GPIO_PC_DIN);

	// Send a signal to other processes that a button was pressed
	kill_fasync(&gp_async_queue, SIGIO, POLL_IN);
	return IRQ_HANDLED;
}

static int gp_open(struct inode *inode, struct file *file_pointer)
{
	return 0;
}

static int gp_release(struct inode *inode, struct file *file_pointer)
{
	return 0;
}

static long gp_ioctl(struct file *file_pointer, unsigned int cmd,
		     unsigned long arg)
{
	switch (cmd) {
	case GP_IOCTL_GET_BUTTON_STATE:
		return gp_button_state;
		break;
	};
	return 0;
}

static int gp_fasync(int fd, struct file *filp, int on)
{
	int temp;
	temp = fasync_helper(fd, filp, on, &gp_async_queue);
	if (fd != -1)
		kill_fasync(&gp_async_queue, SIGIO, POLL_IN);
	return temp;
}

static int register_chr_dev(void)
{
	int ret;
	if ((ret = alloc_chrdev_region(&gp_dev_number, 0, 1, DRIVER_NAME)) < 0) {
		printk(KERN_ERR "ERROR %d: Failed alloc_chr_region %s\n", ret,
		       DRIVER_NAME);
	}
	gp_cdev_ptr = cdev_alloc();
	cdev_init(gp_cdev_ptr, &gp_fops);
	if ((ret = cdev_add(gp_cdev_ptr, gp_dev_number, 1)) < 0) {
		printk(KERN_ERR "ERROR %d: Failed to add cdev %s\n", ret,
		       DRIVER_NAME);
	}
	gp_cl = class_create(THIS_MODULE, DRIVER_NAME);
	device_create(gp_cl, NULL, gp_dev_number, NULL, DRIVER_NAME);

	return 0;
}

static void unregister_chr_dev(void)
{
	unregister_chrdev_region(gp_dev_number, 1);
	cdev_del(gp_cdev_ptr);
	class_destroy(gp_cl);
}

static int gp_probe(struct platform_device *p_dev_ptr)
{
	int gpio_even_irq;
	int gpio_odd_irq;
	struct resource *res;
	int gpio_base_address;
	int ret;
	register_chr_dev();

	// Get info about GPIO
	res = platform_get_resource(p_dev_ptr, IORESOURCE_MEM, 0);
	gpio_base_address = res->start;
	// Get GPIO IRQ number
	gpio_even_irq = platform_get_irq(p_dev_ptr, 0);
	gpio_odd_irq = platform_get_irq(p_dev_ptr, 1);
	setup_gpio((volatile void *)gpio_base_address);
	// Register even interrupt
	ret = request_irq(gpio_even_irq, isr_store_button_state, IRQF_SHARED,
			  "gpio_even", p_dev_ptr);
	if (ret) {
		printk(KERN_ERR "ERROR %d: cannot register IRQ %d\n", ret,
		       gpio_even_irq);
		return -EIO;
	}
	// Register odd interrupt
	ret = request_irq(gpio_odd_irq, isr_store_button_state, IRQF_SHARED,
			  "gpio_odd", p_dev_ptr);
	if (ret) {
		printk(KERN_ERR "ERROR %d: cannot register IRQ %d\n", ret,
		       gpio_odd_irq);
		return -EIO;
	}
	return 0;
}

static int gp_remove(struct platform_device *dev)
{
	int gpio_even_irq;
	int gpio_odd_irq;
	unregister_chr_dev();
	gpio_even_irq = platform_get_irq(dev, 0);
	gpio_odd_irq = platform_get_irq(dev, 1);
	free_irq(gpio_even_irq, dev);
	free_irq(gpio_odd_irq, dev);
	return 0;
}

static int __init gp_init(void)
{
	int ret;
	// Register as platform driver
	if ((ret = platform_driver_register(&gp_driver)) != 0) {
		printk(KERN_ERR
		       "ERROR %d: Failed to register platform device\n", ret);
		return ret;
	}
	return 0;
}

static void __exit gp_cleanup(void)
{
}

MODULE_DEVICE_TABLE(of, gp_of_match);
module_init(gp_init);
module_exit(gp_cleanup);

MODULE_DESCRIPTION("Gamepad driver");
MODULE_LICENSE("GPL");
