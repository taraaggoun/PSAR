#include <linux/module.h>
#include <linux/uaccess.h> // copy_from_user
#include <linux/fs.h> // file_operations
#include <linux/kstrtox.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("");
MODULE_DESCRIPTION("Creates a new character device driver that add a pid in open_pids array");

static int major = 0;
#define OPEN_PIDS_LEN 100
extern pid_t open_pids[OPEN_PIDS_LEN];

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

#define PID_INT_LEN 10
long write_pid(struct file* fd, unsigned int cmd, unsigned long arg)
{
	// Check the magic number of the device
	if(_IOC_TYPE(cmd) != 'N') {
		pr_info("Error on commande type\n");
		return -EINVAL;
	}
	if (cmd != PIDW) {
		pr_info("Error on commande\n");
		return -ENOTTY;
	}

	char buffer[PID_INT_LEN] = {0};
	if (copy_from_user(buffer, (char *)arg, PID_INT_LEN) != 0) {
		pr_info("Error on copu from user\n");
		return -1;
	}

	for (int i = 0; i < 100; i++) {
		if (open_pids[i] == 0) {
			if (kstrtoint(buffer, 10, &open_pids[i]) < 0)
				return -EINVAL;
			pr_info("pid %d added to openpidtable\n", open_pids[i]);
			return 0;
		}
		pr_info("tab[%d] = %d\n", i, open_pids[i]);
	}
	return -ENOMEM; // Table full
}

static struct file_operations fops = { .unlocked_ioctl = write_pid };

static int __init ioctl_init(void)
{
	pr_info("init ioctl open\n");
	major = register_chrdev(0, "openctl", &fops);
	if (major < 0) {
		pr_err("Failed to register character device\n");
		return major;
	}
	pr_info("major %d\n", major);
	return 0;
}
module_init(ioctl_init);

static void __exit ioctl_exit(void)
{
	pr_info("exit ioctl open clt\n");
	unregister_chrdev(major, "openctl");
}
module_exit(ioctl_exit);
