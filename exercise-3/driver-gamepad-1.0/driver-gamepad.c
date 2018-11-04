/*
 * This is a demo Linux kernel module.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
// #include <linux/types.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/signal.h>

#include "driver-gamepad.h"

#define DRIVER_NAME "gamepad"

#define CMU_BASE2 0x400c8000
#define CMU_HFPERCLKEN0  ((volatile __u32*)(CMU_BASE2 + 0x044))
#define CMU2_HFPERCLKEN0_GPIO   (1 << 13)

#define GPIO_BASE    0x40006000
#define GPIO_PA_BASE 0x40006000
#define GPIO_PC_BASE 0x40006048

#define GPIO_PC_MODEL    ((volatile uint32_t*)(GPIO_PC_BASE + 0x04))
#define GPIO_PC_DOUT     ((volatile uint32_t*)(GPIO_PC_BASE + 0x0c))
#define GPIO_PC_DIN      ((volatile uint32_t*)(GPIO_PC_BASE + 0x1c))

#define GPIO_PA_DOUT     ((volatile uint32_t*)(GPIO_PA_BASE + 0x0c))
#define GPIO_PA_MODEH    ((volatile uint32_t*)(GPIO_PA_BASE + 0x08))

#define GPIO_EXTIPSELL ((volatile uint32_t*)(GPIO_BASE + 0x100))
#define GPIO_EXTIFALL  ((volatile uint32_t*)(GPIO_BASE + 0x10c))
#define GPIO_IEN       ((volatile uint32_t*)(GPIO_BASE + 0x110))
#define GPIO_IF        ((volatile uint32_t*)(GPIO_BASE + 0x114))
#define GPIO_IFC       ((volatile uint32_t*)(GPIO_BASE + 0x11c))
// Module variables
int long unsigned gp_owner_pid = -1;
static struct class * gp_cl;
static int gp_major;
static int gp_minor;
struct cdev * gp_cdev_ptr;
dev_t gp_dev_number;
static struct fasync_struct * gp_async_queue;
static int unsigned gp_button_state;


static void setupGPIO(void) {
    // TODO: Implement without hardcoding adresses.
    // Is it possible to get all adresses, or do we have to hardcode offsets?
    int long unsigned current_value;
    int long unsigned new_value;

	// Enable buttons
	// Set GPIO PC to input
    new_value = 0x33333333;
    iowrite32(new_value, GPIO_PC_MODEL);

	// Enable internal pullup
    new_value = 0xFF;
    iowrite32(new_value, GPIO_PC_DOUT);

	// Enable interrupts for GPIO C when its state changes
    new_value = 0x22222222;
    iowrite32(new_value, GPIO_EXTIPSELL);

	// Set up the GPIO to interrupt when a bit changes from 1 to 0 (button pressed)
    new_value = 0xFF;
    iowrite32(new_value, GPIO_EXTIFALL);

	// Set up interrupt generation
    current_value = ioread32(GPIO_IEN);
    new_value = current_value | 0xFF;
    iowrite32(new_value, GPIO_IEN);

	// Clear interrupt flags to avoid interrupt on startup
    current_value = ioread32(GPIO_IFC);
    new_value = current_value | ioread32(GPIO_IF);
    iowrite32(new_value, GPIO_IFC);

    // TODO: Remove usage of LEDs
    // Enable LEDs PA12, PA13 and PA14 for testing
    // We must be careful not to change any of the configurations for these registers
    current_value = ioread32(GPIO_PA_MODEH);
    new_value = current_value | 0x05550000;
    iowrite32(new_value, GPIO_PA_MODEH);

    // turn on LEDs D4-D8 (LEDs are active low)
    current_value = ioread32(GPIO_PA_DOUT);
    new_value = current_value & ~(1 << 12);
    new_value = new_value & ~(1 << 13);
    new_value = new_value & ~(1 << 14);
    iowrite32(new_value, GPIO_PA_DOUT);

}

static irqreturn_t ToggleLeds(int irq, void * dev) {
    int long unsigned current_value;
    int long unsigned new_value;
    current_value = ioread32(GPIO_PA_DOUT);
    new_value = current_value ^ (1 << 12);
    new_value = new_value ^ (1 << 13);
    new_value = new_value ^ (1 << 14);
    iowrite32(new_value, GPIO_PA_DOUT);

	// Clear the interrupt to avoid repeating interrupts
    current_value = ioread32(GPIO_IFC);
    new_value = current_value | ioread32(GPIO_IF);
    iowrite32(new_value, GPIO_IFC);

    // Store the button state
    gp_button_state = ioread32(GPIO_PC_DIN);

    // Send a signal
    kill_fasync(&gp_async_queue, SIGIO, POLL_IN);
    return IRQ_HANDLED;
}

static int gp_open(struct inode * inode, struct file * file_pointer) {
    return 0;
}

static int gp_release(struct inode * inode, struct file * file_pointer) {
    return 0;
}

static long gp_ioctl(struct file * file_pointer, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case GP_IOCTL_GET_BUTTON_STATE:
            return gp_button_state;
            break;
    };
    return 0;
}

static int gp_fasync(int fd, struct file *filp, int on) {
    int temp;
    temp = fasync_helper(fd, filp, on, &gp_async_queue);
    if (fd != -1)
        kill_fasync(&gp_async_queue, SIGIO, POLL_IN);
    return temp;
}

static struct file_operations gp_fops = {
    .owner = THIS_MODULE,
    .open = gp_open,
    .release = gp_release,
    .unlocked_ioctl = gp_ioctl,
    .fasync = gp_fasync,
};

/*
static void PrintPDev(struct platform_device * dev) {
    int i;
    struct resource * res;
    printk("gp_probe called\n");
    printk("gamepad driver registered to device with following information:\n");
    printk("name: %s\n", dev->name);
    printk("id: %d\n", dev->id);
    printk("num resources: %d\n", dev->num_resources);
    printk("Resources:\n");
    for ( i = 0; i < dev->num_resources; ++i ) {
        res = platform_get_resource(dev, IORESOURCE_MEM, i);
        printk("Name: %s\n", res->name);
        printk("Start: %#08X\n", res->start);
        printk("End: %#08X\n", res->end);
    }
}
*/

static int RegisterChrDev(void) {
    int ret;
    if ( (ret = alloc_chrdev_region(&gp_dev_number, 0, 1, DRIVER_NAME)) < 0 ) {
        printk("ERROR %d: Failed alloc_chr_region %s\n", ret, DRIVER_NAME);
    }
    gp_major = MAJOR(gp_dev_number);
    gp_minor = MINOR(gp_dev_number);
    gp_cdev_ptr = cdev_alloc();
    cdev_init(gp_cdev_ptr, &gp_fops);
    if ( (ret = cdev_add(gp_cdev_ptr, gp_dev_number, 1)) < 0 ) {
        printk("ERROR %d: Failed to add cdev %s\n", ret, DRIVER_NAME);
    }
    gp_cl = class_create(THIS_MODULE, DRIVER_NAME);
    device_create(gp_cl, NULL, gp_dev_number, NULL, DRIVER_NAME);

    return 0;
}

static void UnregisterChrDev(void) {
    unregister_chrdev_region(gp_dev_number, 1);
    cdev_del(gp_cdev_ptr);
    class_destroy(gp_cl);
}

static int gp_probe(struct platform_device * p_dev_ptr) {
    int gpio_even_irq;
    int gpio_odd_irq;
    struct resource * res;
    RegisterChrDev();
    // PrintPDev(p_dev_ptr);

    // Get info about GPIO
    res = platform_get_resource(p_dev_ptr, IORESOURCE_MEM, 0);
    // printk("GPIO start: %#08X\n", res->start);
    // printk("GPIO end: %#08X\n", res->end);
    // Get GPIO IRQ number
    gpio_even_irq = platform_get_irq(p_dev_ptr, 0);
    gpio_odd_irq = platform_get_irq(p_dev_ptr, 1);
    // printk("GPIO even IRQ: %d\n", gpio_even_irq);
    // printk("GPIO odd IRQ: %d\n", gpio_odd_irq);
    setupGPIO();
    // Register an interrupt
    if (request_irq(gpio_even_irq, ToggleLeds, IRQF_SHARED, "gpio_even", p_dev_ptr)) {
        printk(KERN_ERR "rtc: cannot register IRQ %d\n", gpio_even_irq);
        return -EIO;
    }
    if (request_irq(gpio_odd_irq, ToggleLeds, IRQF_SHARED, "gpio_even", p_dev_ptr)) {
        printk(KERN_ERR "rtc: cannot register IRQ %d\n", gpio_odd_irq);
        return -EIO;
    }

    // Setup interrupt to send signal
    // TODO
    return 0;
}

static int gp_remove(struct platform_device * dev) {
    int gpio_even_irq;
    int gpio_odd_irq;
    UnregisterChrDev();
    gpio_even_irq = platform_get_irq(dev, 0);
    gpio_odd_irq = platform_get_irq(dev, 1);
    free_irq(gpio_even_irq, dev);
    free_irq(gpio_odd_irq, dev);
    return 0;
}

static const struct of_device_id gp_of_match[] = {
    { .compatible = "tdt4258", },
    { },
};
MODULE_DEVICE_TABLE(of, gp_of_match);

static struct platform_driver gp_driver = {
    .probe = gp_probe,
    .remove = gp_remove,
    .driver = {
        .name = "gp",
        .owner = THIS_MODULE,
        .of_match_table = gp_of_match,
    },
};


/*
 * template_init - function to insert this module into kernel space
 *
 * This is the first of two exported functions to handle inserting this
 * code into a running kernel
 *
 * Returns 0 if successfull, otherwise -1
 */

static int __init template_init(void) {
    int ret;
    // Register as platform driver
    if ( (ret = platform_driver_register(&gp_driver)) != 0 ) {
        printk("Failed to register platform device: %d\n", ret);
        return ret;
    }
    // RegisterChrDev();
	return 0;
}

/*
 * template_cleanup - function to cleanup this module from kernel space
 *
 * This is the second of two exported functions to handle cleanup this
 * code from a running kernel
 */

static void __exit template_cleanup(void) {
}

module_init(template_init);
module_exit(template_cleanup);


MODULE_DESCRIPTION("Gamepad driver");
MODULE_LICENSE("GPL");

