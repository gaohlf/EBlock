#include "EbSys.h"
#include "ECreateBlkDev.h"
#include "EBlockCtrlDev.h"
#include "EBlockRequestManager.h"

#define DEVICE_NAME "eblock"

static int __init EBlock_init(void) {
    int ret = 0;
    printk(KERN_INFO "++++++++++++++++++++++++++EBlock_init start+++++++++++++++++++++++++++++++++++++++++++++++++\n");
    ret = preAllocnEBRequests();
    printk(KERN_INFO "+preAllocnEBRequests ret %d\n", ret);
    if(ret != 0)
    {
        printk(KERN_ERR "+preAllocnEBRequests failed ret %d\n", ret);
        return ret;
    }

    devCtrlInit();
    printk(KERN_INFO "+devCtrlInit\n");

    addEBlockSys();

    printk(KERN_INFO "+addEBlockSys\n");
    ret = createEblockDev(DEVICE_NAME);
    printk(KERN_INFO "+createEblockDev ret %d\n", ret);
    if(ret != 0)
    {
        printk(KERN_ERR "+createEblockDev failed ret %d\n", ret);
        return ret;
    }
    return ret;
}

static void __exit EBlock_exit(void) {
    printk(KERN_INFO "++++++++++++++++++++++++++EBlock_exit start+++++++++++++++++++++++++++++++++++++++++++++++++\n");

    //清理所有创建的块设备
    clearEblockDevs();

    rmEBlockSys();
    devCtrlExit();
    destoryPreAllocEBRequests();
}

module_init(EBlock_init);
module_exit(EBlock_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ivan Gao");
MODULE_DESCRIPTION("A simple Block Kernel Module based on S3");
MODULE_VERSION("1.0");
