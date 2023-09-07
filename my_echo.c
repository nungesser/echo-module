#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/proc_fs.h>
#include <linux/slab.h>

#include <linux/delay.h>

#define MAX_USER_SIZE 100

static struct proc_dir_entry *echo_proc = NULL;
static char   data_buffer[MAX_USER_SIZE];
static int    data_size=0;
static bool   flag          = false;
static bool   about_to_exit = false;

ssize_t echo_read(struct file *file, char __user *user, size_t size, loff_t *off)
{
	while ( flag == false )
	{
		if (about_to_exit || fatal_signal_pending(current) )
			return 0;

		// If there is nothing to give, we wait 200ms before checking again
		msleep(200);
	}
	flag=false;
	return copy_to_user(user, data_buffer, data_size) ? 0 : data_size;
}

ssize_t echo_write(struct file *file, const char __user *user, size_t size, loff_t *off)
{
	memset(data_buffer, 0x0, sizeof(data_buffer));

	if (size > MAX_USER_SIZE)
		data_size=MAX_USER_SIZE;
	else
		data_size=size;
	

	if (copy_from_user(data_buffer, user, data_size))
		return 0;
	flag = true;
	//printk("Data buffer: %s\n", data_buffer);
	return data_size;
}

static const struct proc_ops echo_proc_fops =
{
	.proc_read = echo_read,
	.proc_write = echo_write,
};

static int __init my_echo_init (void)
{
	printk("Welcome to my echo driver!\n");
	
	// create an entry in the proc-fs
	echo_proc = proc_create("my_echo", 0666, NULL, &echo_proc_fops);
	if (echo_proc == NULL)
	{
		return -1;
	}

	return 0;
}

static void __exit my_echo_exit (void)
{
	about_to_exit = true;
	printk("Leaving my echo driver!\n");
	proc_remove(echo_proc);
	return;
}

module_init(my_echo_init);
module_exit(my_echo_exit);


MODULE_AUTHOR("remi.leveque@pm.me");
MODULE_DESCRIPTION("simple echo module");
MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");
