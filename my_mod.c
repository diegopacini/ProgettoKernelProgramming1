#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");

static bool cannot_insert;
module_param(cannot_insert, bool, 0);

static int __init test_module_init(void)
{
    printk("Modulo inserito nel kernel!!!\n");

    if (cannot_insert) {
        return -1;
    }
    return 0;
}

static void __exit test_module_unload(void)
{
    printk("Modulo rimosso dal kernel!!!\n");
}

module_init(test_module_init);
module_exit(test_module_unload);
