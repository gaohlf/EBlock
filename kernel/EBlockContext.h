#ifndef EBLOCKCONTEXT_H
#define EBLOCKCONTEXT_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/hashtable.h>
#include <linux/dcache.h> // 包含full_name_hash的头文件
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/mutex.h>

#include "../include/EBlockRequests.h"

struct EBlockContext
{
    char name[BLOCK_CONTEXT_NAME_SIZE];//EBLOCK 主键名称
    //每个的disk应该有一个独立的queue、gd和data
    struct request_queue *queue;
    struct gendisk *gd;
    struct hlist_node hash_node;
};

//定义一个回调函数用来clear中销毁EBlockContext中的结构
typedef void (*destroyBlockContext_t)(struct EBlockContext *);

//申请newEBlockContext空间 并插入全局hash表
extern struct EBlockContext * newEBlockContext(const char *key);
//根据key找到EBlockContext
extern struct EBlockContext *findEBlockContext(const char *key);
//删除指定主键的设备只是销毁内存
extern void removeEBlockContext(const char *key);
//清空接口并销毁其中相应结构
extern void clearEBlockContexts(destroyBlockContext_t destroyBlockContext_cb);

#endif