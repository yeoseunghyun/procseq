
#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module */
#include <linux/proc_fs.h>	/* Necessary because we use proc fs */
#include <linux/seq_file.h>	/* for seq_file */
#include <linux/slab.h>
#include <linux/uaccess.h>

#define PROC_NAME	"yeo14"

MODULE_AUTHOR("byyeo14");
MODULE_LICENSE("GPL");

#define BUFSIZE 512
static char* inbuf;
static unsigned long buffersize =0;

/**
 * This function is called at the beginning of a sequence.
 * ie, when:
 *	- the /proc file is read (first time)
 *	- after the function stop (end of sequence)
 *
 */
static void *my_seq_start(struct seq_file *s, loff_t *pos)
{
	static unsigned long counter = 0;

	/* beginning a new sequence ? */	
	if ( *pos == 0 )
	{	
		/* yes => return a non null value to begin the sequence */
		return &counter;
	}
	else
	{
		/* no => it's the end of the sequence, return end to stop reading */
		*pos = 0;
		return NULL;
	}
}

/**
 * This function is called after the beginning of a sequence.
 * It's called untill the return is NULL (this ends the sequence).
 *
 */
static void *my_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	unsigned long *tmp_v = (unsigned long *)v;
	(*tmp_v)++;
	(*pos)++;
	return NULL;
}

/**
 * This function is called at the end of a sequence
 * 
 */
static void my_seq_stop(struct seq_file *s, void *v)
{
	/* nothing to do, we use a static value in start() */
}

/**
 * This function is called for each "step" of a sequence
 *
 */
static int my_seq_show(struct seq_file *s, void *v)
{
	loff_t *spos = (loff_t *) v;

	seq_printf(s, "%s\n", inbuf);
	return 0;
}

/**
 * This structure gather "function" to manage the sequence
 *
 */
static struct seq_operations my_seq_ops = {
	.start = my_seq_start,
	.next  = my_seq_next,
	.stop  = my_seq_stop,
	.show  = my_seq_show
};

/**
 * This function is called when the /proc file is open.
 *
 */
static int my_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &my_seq_ops);
};

/**
 * This structure gather "function" that manage the /proc file
 *
 */
static ssize_t my_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
	if(count > BUFSIZE)
		buffersize= BUFSIZE;
	else buffersize=count;

	if(copy_from_user(inbuf, ubuf, buffersize))
	{
		printk(KERN_ALERT "copy from user in mywrit  error\n");
		return -EFAULT;
	}
	return buffersize;
}


static struct file_operations my_file_ops = {
	.owner   = THIS_MODULE,
	.open    = my_open,
	.write   = my_write,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release
};


/**
 * This function is called when the module is loaded
 *
 */
int init_module(void)
{
	struct proc_dir_entry *entry;
	inbuf=kmalloc(BUFSIZE,GFP_KERNEL);
	entry = proc_create(PROC_NAME, 0, NULL,&my_file_ops);
	return 0;
}

/**
 * This function is called when the module is unloaded.
 *
 */
void cleanup_module(void)
{
	remove_proc_entry(PROC_NAME, NULL);
	kfree(inbuf);
}

