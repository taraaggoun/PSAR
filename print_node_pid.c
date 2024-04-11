#include "linux/module.h"
#include <linux/fdtable.h>
#include "linux/pagemap.h"
#include "linux/xarray.h"

MODULE_AUTHOR("");
MODULE_DESCRIPTION("Print the nodes where the path cache contain files open in cpus");
MODULE_LICENSE("GPL");

static int pid = 0;
module_param(pid, int, 0644);
MODULE_PARM_DESC(pid, "pid of process");

static int nb_fds = 100;
static int fds[100] = { 0 };
module_param_array(fds, int, &nb_fds, 0);
MODULE_PARM_DESC(fds, "array with files descriptor");

struct fdtable* get_fdtable(void)
{
	// Get process's struct pid
	struct pid *s_pid = find_get_pid(pid);
	if (!s_pid) {
		pr_err("Failed to get pid\n");
		return NULL;
	}

	// Get process's struct task
	struct task_struct *task = get_pid_task(s_pid, PIDTYPE_PID);
	if (!task) {
		pr_err("Failed to get struct task\n");
		return NULL;
	}

	// Get process's struct files
	struct files_struct *files = task->files;
	if (!files) {
		pr_err("Failed to get files_struct\n");
		return NULL;
	}

	// Get process's fd table
	struct fdtable *fdt;
	rcu_read_lock();
	fdt = files_fdtable(files);
	rcu_read_unlock();
	if (!fdt) {
		pr_err("file table NULL\n");
		return NULL;
	}

	// Free memory
	put_task_struct(task);
	put_pid(s_pid);

	return fdt;
}

int print_stat(struct fdtable *fdt, int id)
{
	if (fds[id] < 0 || fds[id] >= fdt->max_fds) {
		pr_err("Invalid file descriptor: %d\n", fds[id]);
		return 1;
	}
	struct file *file = NULL;
	file = fdt->fd[fds[id]];
	if (file == NULL)
		return 1;

	struct address_space *mapping = file->f_inode->i_mapping;
	
	if (xa_empty(&(mapping->i_pages)) || mapping->nrpages <= 0) {
		pr_info("File 1 not loaded in memory\n");
		return 0;
	}

	int tab[NR_CPUS] = { 0 };
	int nb_folio = 0;

	xa_lock(&mapping->i_pages);
	XA_STATE(xas, &mapping->i_pages, 0);
	
	struct folio *folio;
	pr_info("File %d in node :\n", id);
	
	// Go throught all folio
	rcu_read_lock();
	xas_for_each(&xas, folio, ULONG_MAX) {
		tab[folio_nid(folio)]++;
		nb_folio++;
	}
	rcu_read_unlock();
	xa_unlock(&mapping->i_pages);

	for (int i = 0; i < NR_CPUS; i++) {
		if (tab[i] == 0)
			continue;
		pr_info("Core %d : %d%%\n", i, tab[i] / nb_folio * 100);
	}
	return 0;
}

static int __init print_nodes_pid_init(void)
{
	pr_info("Print nodes module 2 init\n");
	// If no pid or fd
	if (!pid || !fds[0])
		return 1;

	struct fdtable *fdt = get_fdtable();
	if (fdt == NULL) return 1;

	for (int i = 0; i < 100; i++) {
		if (fds[i] == 0)
			break;
		if (print_stat(fdt, i) == 1)
			return 1;
	}
	return 0;
}

module_init(print_nodes_pid_init);

static void __exit print_nodes_pid_exit(void)
{
	pr_info("Print nodes module 2 cleaned up\n");
}
module_exit(print_nodes_pid_exit); 