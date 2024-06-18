//
// Created by 20075 on 2024/5/31.
//
#include "EbSys.h"
#include "EBlockRequestManager.h"
#include "EBlockRequestQueue.h"
#include "EBlockPendingMap.h"
#include "EBlockStatics.h"

#define PROC_FILENAME "EBlock"

static struct proc_dir_entry *proc_file;
static char proc_data[1024];
static int proc_data_len;

static ssize_t proc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    if (*ppos > 0 || count < proc_data_len)
    return 0;

    if (copy_to_user(buf, proc_data, proc_data_len))
    return -EFAULT;

    *ppos = proc_data_len;
    return proc_data_len;
}


static void dealCmd(const char *cmd)
{
    if(strcmp(cmd, "dump requests") == 0 || strcmp(cmd, "dr"))
    {
        dumpAllPendingEBRequests();
        dumpAllQueueRequests();
        dumpAllStatics();
    }
}

static ssize_t proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    if (count > sizeof(proc_data) - 1)
    return -EINVAL;

    if (copy_from_user(proc_data, buf, count))
    return -EFAULT;

    proc_data[count] = '\0';
    proc_data_len = count;

    dealCmd(proc_data);

    return count;
}

static const struct file_operations proc_fops = {
        .owner = THIS_MODULE,
        .read = proc_read,
        .write = proc_write,
};

void addEBlockSys(void)
{
    printk(KERN_INFO "EBlock addEBlockSys: start\n");
    proc_file = proc_create(PROC_FILENAME, 0666, NULL, &proc_fops);
    if (!proc_file) {
        printk(KERN_ERR "EBlock addEBlockSys: proc file created failed\n");
        return;
    }
    printk(KERN_INFO "EBlock addEBlockSys: proc file created\n");
}

void rmEBlockSys(void)
{
    proc_remove(proc_file);
    printk(KERN_INFO "EBlock rmEBlockSys: proc file removed\n");
}

EXPORT_SYMBOL(addEBlockSys);
EXPORT_SYMBOL(rmEBlockSys);