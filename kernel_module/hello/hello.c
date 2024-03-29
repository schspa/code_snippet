#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

static int hello_init(void)
{
	printk(KERN_ALERT "Hello, world\n");
	return 0;
}

static void hello_exit(void)
{
	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_AUTHOR("iamcopper<kangpan519@gmail.com>");
MODULE_LICENSE("GPL v2");
