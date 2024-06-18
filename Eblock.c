#include "EbSys.h"

static int __init EBlock_init(void) {
    printk(KERN_INFO "Hello, World!\n");
    addEBlockSys();
    return 0;
}

static void __exit EBlock_exit(void) {
    printk(KERN_INFO "Goodbye, World!\n");
    rmEBlockSys();
}

module_init(EBlock_init);
module_exit(EBlock_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ivan Gao");
MODULE_DESCRIPTION("A simple Hello World Kernel Module");
MODULE_VERSION("1.0");
