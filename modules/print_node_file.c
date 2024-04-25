#include "linux/module.h"
#include "linux/pagemap.h"
#include "linux/fs.h"
#include "linux/xarray.h"

MODULE_AUTHOR("");
MODULE_DESCRIPTION("Print the nodes where the path cache contain the file");
MODULE_LICENSE("GPL");

static char *filename = NULL;
module_param(filename, charp, 0644);
MODULE_PARM_DESC(filename, "Name of the file");

void print_nid(void)
{
	// Open the file in read only
	struct file *file = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(file)) {
		pr_info("Failed to open file\n");
		return;
	}

	struct folio *folio;
	struct address_space *mapping = file->f_inode->i_mapping;

	if (xa_empty(&(mapping->i_pages)) || mapping->nrpages <= 0) {
		pr_info("File not loaded in memory\n");
		filp_close(file, NULL);
		return;
	}

	xa_lock(&mapping->i_pages);
	XA_STATE(xas, &mapping->i_pages, 0);

	pr_info("File in node :\n");
	// Go throught all folio
	rcu_read_lock();
	xas_for_each(&xas, folio, ULONG_MAX) {
		// Print the nid's number
		pr_info("nid : %d \n", folio_nid(folio));
		// We are done with this struct folio
		folio_put(folio);
	}
	rcu_read_unlock();
	xa_unlock(&mapping->i_pages);

	// Close the file
	filp_close(file, NULL);
}

static int __init print_nodes_pid_init(void)
{
	if (filename == NULL) {
		pr_info("No filename provided\n");
		return -EINVAL;
	}
	pr_info("Print nodes module of %s\n", filename);
		print_nid();
		
	return 0;
}
module_init(print_nodes_pid_init);

static void __exit print_nodes_pid_exit(void)
{
	pr_info("Print nodes module cleaned up\n");
}
module_exit(print_nodes_pid_exit);
