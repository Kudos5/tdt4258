/*
 * This is a demo Linux kernel module.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>

static int gp_probe(struct platform_device * dev) {
    printk("gp_probe called\n");
    return 0;
}

static int gp_remove(struct platform_device * dev) {
    printk("gp_remove called\n");
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

