/*
 * The proc filesystem constants/structures
 */
#ifndef __PROCFS_INTERNAL_H
#define __PROCFS_INTERNAL_H

#include <linux/magic.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>

struct net;
struct mm_struct;
struct pid_namespace;

/*
 * Offset of the first process in the /proc root directory..
 */
#define FIRST_PROCESS_ENTRY 256

/* Worst case buffer size needed for holding an integer. */
#define PROC_NUMBUF 13

/*
 * We always define these enumerators
 */
enum {
	PROC_ROOT_INO = 1,
};

enum kcore_type {
	KCORE_TEXT,
	KCORE_VMALLOC,
	KCORE_RAM,
	KCORE_VMEMMAP,
	KCORE_OTHER,
};

struct kcore_list {
	struct list_head list;
	unsigned long addr;
	size_t size;
	int type;
};

struct vmcore {
	struct list_head list;
	unsigned long long paddr;
	unsigned long long size;
	loff_t offset;
};

#ifdef CONFIG_PROC_FS
extern void proc_root_init(void);

void proc_flush_task(struct task_struct *task);

extern int pid_ns_prepare_proc(struct pid_namespace *ns);
extern void pid_ns_release_proc(struct pid_namespace *ns);

/*
 * proc_tty.c
 */
struct tty_driver;
extern void proc_tty_init(void);
extern void proc_tty_register_driver(struct tty_driver *driver);
extern void proc_tty_unregister_driver(struct tty_driver *driver);

/*
 * proc_devtree.c
 */
#ifdef CONFIG_PROC_DEVICETREE
struct device_node;
struct property;
extern void proc_device_tree_init(void);
extern void proc_device_tree_add_node(struct device_node *, struct proc_dir_entry *);
extern void proc_device_tree_add_prop(struct proc_dir_entry *pde, struct property *prop);
extern void proc_device_tree_remove_prop(struct proc_dir_entry *pde,
					 struct property *prop);
extern void proc_device_tree_update_prop(struct proc_dir_entry *pde,
					 struct property *newprop,
					 struct property *oldprop);
#endif /* CONFIG_PROC_DEVICETREE */

/* Legacy read_proc implementations, don't use in new code */
extern ssize_t proc_file_read(struct file *file, char __user *buf,
				 size_t nbytes, loff_t *ppos);
#else

static inline void proc_flush_task(struct task_struct *task)
{
}

static inline int pid_ns_prepare_proc(struct pid_namespace *ns)
{
	return 0;
}

static inline void pid_ns_release_proc(struct pid_namespace *ns)
{
}

struct tty_driver;
static inline void proc_tty_register_driver(struct tty_driver *driver) {};
static inline void proc_tty_unregister_driver(struct tty_driver *driver) {};

static inline ssize_t proc_file_read(struct file *file, char __user *buf,
				     size_t nbytes, loff_t *ppos)
{
	return 0;
}
#endif

#if !defined(CONFIG_PROC_KCORE)
static inline void
kclist_add(struct kcore_list *new, void *addr, size_t size, int type)
{
}
#else
extern void kclist_add(struct kcore_list *, void *, size_t, int type);
#endif

union proc_op {
	int (*proc_get_link)(struct inode *, struct path *);
	int (*proc_read)(struct task_struct *task, char *page);
	int (*proc_show)(struct seq_file *m,
		struct pid_namespace *ns, struct pid *pid,
		struct task_struct *task);
};

struct ctl_table_header;
struct ctl_table;

struct proc_inode {
	struct pid *pid;
	int fd;
	union proc_op op;
	struct proc_dir_entry *pde;
	struct ctl_table_header *sysctl;
	struct ctl_table *sysctl_entry;
	void *ns;
	const struct proc_ns_operations *ns_ops;
	struct inode vfs_inode;
};

static inline struct proc_inode *PROC_I(const struct inode *inode)
{
	return container_of(inode, struct proc_inode, vfs_inode);
}

static inline struct proc_dir_entry *PDE(const struct inode *inode)
{
	return PROC_I(inode)->pde;
}

static inline struct net *PDE_NET(struct proc_dir_entry *pde)
{
	return pde->pde_parent->pde_data;
}

#endif /* __PROCFS_INTERNAL_H */
