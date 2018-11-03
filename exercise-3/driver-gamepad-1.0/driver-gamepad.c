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

#define CMU_BASE2 0x400c8000
#define CMU_HFPERCLKEN0  ((volatile __u32*)(CMU_BASE2 + 0x044))
#define CMU2_HFPERCLKEN0_GPIO   (1 << 13)

#define GPIO_BASE    0x40006000
#define GPIO_PA_BASE 0x40006000
#define GPIO_PC_BASE 0x40006048

#define GPIO_PC_MODEL    ((volatile uint32_t*)(GPIO_PC_BASE + 0x04))
#define GPIO_PC_DOUT     ((volatile uint32_t*)(GPIO_PC_BASE + 0x0c))

#define GPIO_PA_DOUT     ((volatile uint32_t*)(GPIO_PA_BASE + 0x0c))
#define GPIO_PA_MODEH    ((volatile uint32_t*)(GPIO_PA_BASE + 0x08))

#define GPIO_EXTIPSELL ((volatile uint32_t*)(GPIO_BASE + 0x100))
#define GPIO_EXTIFALL  ((volatile uint32_t*)(GPIO_BASE + 0x10c))
#define GPIO_IEN       ((volatile uint32_t*)(GPIO_BASE + 0x110))
#define GPIO_IF        ((volatile uint32_t*)(GPIO_BASE + 0x114))
#define GPIO_IFC       ((volatile uint32_t*)(GPIO_BASE + 0x11c))
static void setupGPIO(void) {
    // TODO: Implement without hardcoding adresses.
    // Is it possible to get all adresses, or do we have to hardcode offsets?
    int long unsigned current_value;
    int long unsigned new_value;
    // enable GPIO clock
	// *CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_GPIO;	
    // current_value = ioread32(CMU_HFPERCLKEN0);
    // new_value = current_value | CMU2_HFPERCLKEN0_GPIO;
    // iowrite32(new_value, CMU_HFPERCLKEN0);

	// Enable buttons
	// Set GPIO PC to input
	// *GPIO_PC_MODEL = 0x33333333;
    new_value = 0x33333333;
    iowrite32(new_value, GPIO_PC_MODEL);
	// Enable internal pullup
	// *GPIO_PC_DOUT = 0xFF;

    new_value = 0xFF;
    iowrite32(new_value, GPIO_PC_DOUT);
	// Enable interrupts for GPIO C when its state changes
	// *GPIO_EXTIPSELL = 0x22222222;
    new_value = 0x22222222;
    iowrite32(new_value, GPIO_EXTIPSELL);
	// Set up the GPIO to interrupt when a bit changes from 1 to 0 (button pressed)
	// *GPIO_EXTIFALL = 0xFF;
    new_value = 0xFF;
    iowrite32(new_value, GPIO_EXTIFALL);
	// Set up interrupt generation
	// *GPIO_IEN |= 0xFF;
    current_value = ioread32(GPIO_IEN);
    new_value = current_value | 0xFF;
    iowrite32(new_value, GPIO_IEN);
	// Clear interrupt flags to avoid interrupt on startup
	// *GPIO_IFC = *GPIO_IF;

    // Enable LEDs PA12, PA13 and PA14 for testing
    // We must be careful not to change any of the configurations for these registers
    // *GPIO_PA_CTRL = 2;		/* set high drive strength */
    // *GPIO_PA_MODEH = 0x55555555;	/* set pins A8-15 as output */
    current_value = ioread32(GPIO_PA_MODEH);
    new_value = current_value | 0x05550000;
    iowrite32(new_value, GPIO_PA_MODEH);
    // *GPIO_PA_DOUT = 0x0700;	/* turn on LEDs D4-D8 (LEDs are active low) */
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
	// *GPIO_IFC |= *GPIO_IF;
    current_value = ioread32(GPIO_IFC);
    new_value = current_value | ioread32(GPIO_IF);
    iowrite32(new_value, GPIO_IFC);

    return IRQ_HANDLED;
}

static int gp_probe(struct platform_device * dev) {
    int gpio_even_irq;
    int gpio_odd_irq;
    struct resource * res;
    int i;
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

    // Get info about GPIO
    res = platform_get_resource(dev, IORESOURCE_MEM, 0);
    printk("GPIO start: %#08X\n", res->start);
    printk("GPIO end: %#08X\n", res->end);
    // Get GPIO IRQ number
    gpio_even_irq = platform_get_irq(dev, 0);
    gpio_odd_irq = platform_get_irq(dev, 1);
    printk("GPIO even IRQ: %d\n", gpio_even_irq);
    printk("GPIO odd IRQ: %d\n", gpio_odd_irq);
    setupGPIO();
    // Register an interrupt
    if (request_irq(gpio_even_irq, ToggleLeds, IRQF_SHARED, "gpio_even", dev)) {
        printk(KERN_ERR "rtc: cannot register IRQ %d\n", gpio_even_irq);
        return -EIO;
    }
    if (request_irq(gpio_odd_irq, ToggleLeds, IRQF_SHARED, "gpio_even", dev)) {
        printk(KERN_ERR "rtc: cannot register IRQ %d\n", gpio_odd_irq);
        return -EIO;
    }
    return 0;
}

static int gp_remove(struct platform_device * dev) {
    int gpio_even_irq;
    int gpio_odd_irq;
    printk("gp_remove called\n");
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

static int __init template_init(void)
{
	printk("Hello World, here is your module speaking\n");
    platform_driver_register(&gp_driver);
	return 0;
}

/*
 * template_cleanup - function to cleanup this module from kernel space
 *
 * This is the second of two exported functions to handle cleanup this
 * code from a running kernel
 */

static void __exit template_cleanup(void)
{
	 printk("Short life for a small module...\n");
}

module_init(template_init);
module_exit(template_cleanup);


MODULE_DESCRIPTION("Small module, demo only, not very useful.");
MODULE_LICENSE("GPL");

