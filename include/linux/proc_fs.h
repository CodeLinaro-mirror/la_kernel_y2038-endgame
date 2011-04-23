/*
 * The proc filesystem interface for device drivers
 */

#ifndef _LINUX_PROC_FS_H
#define _LINUX_PROC_FS_H

#include <linux/slab.h> /* not needed by this file, please remove */
#include <linux/fs.h>
#include <asm/atomic.h>

struct net;
struct proc_dir_entry;
typedef	int (read_proc_t)(char *page, char **start, off_t off,
			  int count, int *eof, void *data);

#ifdef CONFIG_PROC_FS
struct proc_dir_entry *proc_create_size(const char *name, mode_t mode,
				struct proc_dir_entry *parent,
				const struct file_operations *proc_fops,
				void *data, size_t size);
extern void remove_proc_entry(const char *name, struct proc_dir_entry *parent);

extern struct proc_dir_entry *proc_symlink(const char *,
		struct proc_dir_entry *, const char *);
extern struct proc_dir_entry *proc_mkdir(const char *,struct proc_dir_entry *);
extern struct proc_dir_entry *proc_mkdir_mode(const char *name, mode_t mode,
			struct proc_dir_entry *parent);

static inline struct proc_dir_entry *proc_create(const char *name, mode_t mode,
	struct proc_dir_entry *parent, const struct file_operations *proc_fops)
{
	return proc_create_size(name, mode, parent, proc_fops, NULL, 0);
}

static inline struct proc_dir_entry *proc_create_data(const char *name,
	mode_t mode, struct proc_dir_entry *parent,
	const struct file_operations *proc_fops, void *data)
{
	return proc_create_size(name, mode, parent, proc_fops, data, 0);
}

extern void proc_remove(struct proc_dir_entry *pde);

extern struct proc_dir_entry *create_proc_read_entry(const char *name,
	mode_t mode, struct proc_dir_entry *base, 
	read_proc_t *read_proc, void * data);
 
extern struct proc_dir_entry *proc_net_fops_create(struct net *net,
	const char *name, mode_t mode, const struct file_operations *fops);
extern void proc_net_remove(struct net *net, const char *name);
extern struct proc_dir_entry *proc_net_mkdir(struct net *net, const char *name,
	struct proc_dir_entry *parent);

extern struct file *proc_ns_fget(int fd);

#else

#define proc_net_fops_create(net, name, mode, fops)  ({ (void)(mode), NULL; })
static inline void proc_net_remove(struct net *net, const char *name) {}

static inline struct proc_dir_entry *proc_create(const char *name,
	mode_t mode, struct proc_dir_entry *parent,
	const struct file_operations *proc_fops)
{
	return NULL;
}
static inline struct proc_dir_entry *proc_create_data(const char *name,
	mode_t mode, struct proc_dir_entry *parent,
	const struct file_operations *proc_fops, void *data)
{
	return NULL;
}
static inline struct proc_dir_entry *proc_create_size(const char *name,
	mode_t mode, struct proc_dir_entry *parent,
	const struct file_operations *proc_fops, void *data, size_t size)
{
	return NULL;
}
#define remove_proc_entry(name, parent) do {} while (0)

static inline struct proc_dir_entry *proc_symlink(const char *name,
		struct proc_dir_entry *parent,const char *dest) {return NULL;}
static inline struct proc_dir_entry *proc_mkdir(const char *name,
	struct proc_dir_entry *parent) {return NULL;}
static inline struct proc_dir_entry *proc_mkdir_mode(const char *name,
	mode_t mode, struct proc_dir_entry *parent) { return NULL; }

static inline void proc_remove(struct proc_dir_entry *pde) { return NULL; }

static inline struct proc_dir_entry *create_proc_read_entry(const char *name,
	mode_t mode, struct proc_dir_entry *base, 
	read_proc_t *read_proc, void * data) { return NULL; }

static inline struct file *proc_ns_fget(int fd)
{
	return ERR_PTR(-EINVAL);
}

#endif /* CONFIG_PROC_FS */

struct nsproxy;
struct proc_ns_operations {
	const char *name;
	int type;
	void *(*get)(struct task_struct *task);
	void (*put)(void *ns);
	int (*install)(struct nsproxy *nsproxy, void *ns);
};
extern const struct proc_ns_operations netns_operations;
extern const struct proc_ns_operations utsns_operations;
extern const struct proc_ns_operations ipcns_operations;

#endif /* _LINUX_PROC_FS_H */
