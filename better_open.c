#include <linux/mm.h>
#include <linux/fdtable.h>
#include <linux/rcupdate.h>

static struct fdtable* get_fdtable(void)
{
	// Get process's files struct
	struct files_struct *files = current->files;
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
		pr_err("fd table NULL\n");
		return NULL;
	}
	return fdt;
}

static int max_tab(int *tab, int len) {
	int max = tab[0];
	for (int i = 1; i < len; i++)
		if (tab[i] > tab[max])
			max = i;
	return max;
}

static int get_core_fd(struct fdtable *fdt, int fd, int *tab, int len)
{
	struct file *file = NULL;
	file = fdt->fd[fd];
	if (file == NULL)
		return -1;

	struct address_space *mapping = file->f_inode->i_mapping;
	if (xa_empty(&(mapping->i_pages)) || mapping->nrpages <= 0)
		return -1;

	xa_lock(&mapping->i_pages);
	XA_STATE(xas, &mapping->i_pages, 0);

	// Go throught all folio
	struct folio *folio;
	xas_for_each(&xas, folio, ULONG_MAX) {
		// Add the number of pages in tab
		tab[folio_nid(folio)] += folio_nr_pages(folio);
	}
	xa_unlock(&mapping->i_pages);
	
	return 0;
}

static int get_core_pid(int *tab, int len)
{
	struct fdtable *fdt = get_fdtable();
	if (fdt == NULL)
		return -1;
	for (int i = 3; i < fdt->max_fds; i++) {
		if (get_core_fd(fdt, i, tab, len) == -1)
			return -1;
	}
	return max_tab(tab, len);
}

void mv_core(void)
{
	cpumask_t cpu_mask = { 0 };
	int tab[NR_CPUS] = { 0 };
	
	int core_id = get_core_pid(tab, NR_CPUS);
	if (core_id == -1)
		return;
	cpumask_set_cpu(core_id, &cpu_mask);

	// Assign the program to all the cores in the set
	pr_info(" Go to core : %d\n", core_id);
	if (sched_setaffinity(current->pid, &cpu_mask) == -1)
		pr_info("Error on setaffinity\n");
}