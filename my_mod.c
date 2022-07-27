#include <linux/poll.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/sched.h>


MODULE_AUTHOR("Diego Pacini");
MODULE_DESCRIPTION("Comunicazione sincrona");
MODULE_LICENSE("GPL");

static char *my_pointer;
static int my_len;
static struct mutex my_mutex;
static wait_queue_head_t my_waitqueue_read;
static wait_queue_head_t my_waitqueue_write;

static int my_open(struct inode *inode, struct file *file)
{
  int *count;

  count = kmalloc(sizeof(int), GFP_USER);
  if (count == NULL) {
    return -1;
  }

  *count = 0;
  file->private_data = count;
  
  return 0;
}

static int my_close(struct inode *inode, struct file *file)
{
  kfree(file->private_data);

  return 0;
}

ssize_t my_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
  int res, err;
  int *count;

  mutex_lock(&my_mutex);
  count = file->private_data;
  if (*count == 1) {
    mutex_unlock(&my_mutex);

    return 0;
  }
  /* chi scrive resta bloccato finché qualcuno non ha scritto */
  while (my_pointer == NULL) {
    mutex_unlock(&my_mutex);
    wait_event_interruptible(my_waitqueue_read, (my_pointer != NULL));
    if(signal_pending(current)){
      return -ERESTARTSYS;
    }
    mutex_lock(&my_mutex);
  }
  if (len > my_len) {
    res = my_len;
  } else {
    res = len;
  }
  err = copy_to_user(buf, my_pointer, res);
  if (err) {
    mutex_unlock(&my_mutex);

    return -EFAULT;
  }

  kfree(my_pointer);
  my_pointer = NULL;
  *count = 1;
  /* avvenuta la lettura, vengono svegliati eventuali processi scrittori in coda */
  wake_up_interruptible(&my_waitqueue_write);
  mutex_unlock(&my_mutex);

  return res;
}

static ssize_t my_write(struct file *file, const char __user * buf, size_t count, loff_t *ppos)
{
  int err;
  
  mutex_lock(&my_mutex);
  /* chi scrive resta bloccato finché qualcuno non ha letto */
  while (my_pointer != NULL) {
    mutex_unlock(&my_mutex);
    wait_event_interruptible(my_waitqueue_write, (my_pointer == NULL));
    if(signal_pending(current)){
      return -ERESTARTSYS;
    }
    mutex_lock(&my_mutex);
  }
  my_pointer = kmalloc(count, GFP_USER);
  if (my_pointer == NULL) {
    mutex_unlock(&my_mutex);

    return -1;
  }
  my_len = count;

  err = copy_from_user(my_pointer, buf, count);
  if (err) {
    mutex_unlock(&my_mutex);

    return -EFAULT;
  }
  /* avvenuta la scrittura, vengono svegliati eventuali processi lettori in coda */
  wake_up_interruptible(&my_waitqueue_read);
  mutex_unlock(&my_mutex);
  return count;
}

static struct file_operations my_fops = {
  .owner =        THIS_MODULE,
  .read =         my_read,
  .open =         my_open,
  .release =      my_close,
  .write =        my_write,
#if 0
  .poll =         my_poll,
  .fasync =       my_fasync,
#endif
};

static struct miscdevice test_device = {
  MISC_DYNAMIC_MINOR, "test", &my_fops
};


static int testmodule_init(void)
{
  int res;
  saved_struct = NULL;

  res = misc_register(&test_device);

  printk("Misc Register returned %d\n", res);

  mutex_init(&my_mutex);
  init_waitqueue_head(&my_waitqueue_read);
  init_waitqueue_head(&my_waitqueue_write);

  return 0;
}

static void testmodule_exit(void)
{
  misc_deregister(&test_device);
}

module_init(testmodule_init);
module_exit(testmodule_exit);
