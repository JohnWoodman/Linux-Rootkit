#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Noop Noop");
MODULE_DESCRIPTION("Senior Project");
MODULE_VERSION("0.1");

static int __init rootkit_init(void)
{
		// Here, install initialization procedures
		printk(KERN_INFO "Initializing...\n");
		// TODO: hook syscall table, reset some function pointers
		// TODO: find malware process and remove from process list
		return 0;
}

static void __exit rootkit_exit(void)
{
		// Here, install exit procedures for the rootkit
		printk(KERN_INFO "Exiting...\n");
}

module_init(rootkit_init);
module_exit(rootkit_exit);
