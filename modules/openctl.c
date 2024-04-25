#include "linux/module.h"
#include "linux/kernel.h"
#include "linux/init.h"
#include "linux/sysfs.h"
#include "linux/fs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("");
MODULE_DESCRIPTION("Creates a new character device driver named pids_open");

static int major = 0;
extern pid_t *open_pids;

#define IOCTL_MAGIC 'N'
#define PIDW _IOW(IOCTL_MAGIC, 0, char*)

/**
 * Verifies if a character represents a digit.
 */
int is_num(char c)
{
	return (c >= '0' && c <= '9');
}

/**
 * Converts a string of characters to an integer.
 */
int str_to_int(const char *buffer, size_t len)
{
	int res = 0;
	for (int i = 0; i < len; i++) {
		if (!is_num(buffer[i]))
			return -1;
		res *= 10;
		res += '0' + buffer[i];
	}
	return res;
}

long write_ioctl(struct file *file, const char __user *buffer, size_t len, loff_t *off)
{
	for (int i = 0; i < 100; i++) {
		if (open_pids[i] == 0) {
			open_pids[i] = str_to_int(buffer, len);
			if (open_pids[i] == -1) {
				open_pids[i] = 0;
				return 1;
			}
			pr_info("pid %d added to openpidtable\n", open_pids[i]);
			break;
		}
	}
	return 0;
}

static struct file_operations fops = { .write = write_ioctl };

static int __init ioctl_init(void)
{
	pr_info("init ioctl open\n");
	major = register_chrdev(0, "tab_open_pid", &fops);
	pr_info("major %d\n", major);
	return 0;
}
module_init(ioctl_init);

static void __exit ioctl_exit(void)
{
	pr_info("exit ioctl open clt\n");
	unregister_chrdev(major, "tab_open_pid");
}
module_exit(ioctl_exit);
