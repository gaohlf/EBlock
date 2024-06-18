#ifndef EBSYS_H
#define EBSYS_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

extern void addEBlockSys(void);

extern void rmEBlockSys(void);
#endif