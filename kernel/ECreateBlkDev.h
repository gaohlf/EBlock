#ifndef ECREATEBLKDEV_H
#define ECREATEBLKDEV_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/genhd.h>
#include <linux/errno.h>
#include <linux/vmalloc.h>
#include <linux/hdreg.h>
#include <linux/bio.h>

//创建一个块设备
extern int  createEblockDev(const char * name);
//卸载一个块设备
extern void destoryEblockDev(const char * name);
//卸载所有块设备
extern void clearEblockDevs(void);
#endif