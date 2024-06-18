#include "EBlockContext.h"

#define HASH_BITS_NUM 16
static DEFINE_HASHTABLE(BlockContexts, HASH_BITS_NUM);
static DEFINE_MUTEX(mutex_context);

//申请 newBlock的空间并将其插入 全局hash表中
struct EBlockContext * newEBlockContext(const char *key)
{
    unsigned long hash;
    struct EBlockContext *new_entry = kmalloc(sizeof(*new_entry), GFP_KERNEL);
    if (!new_entry)
        return NULL;

    strncpy(new_entry->name, key, sizeof(new_entry->name) - 1);
    new_entry->name[sizeof(new_entry->name) - 1] = '\0';

    hash = full_name_hash(key, strlen(key));
    mutex_lock(&mutex_context);
    hash_add(BlockContexts, &new_entry->hash_node, hash);
    mutex_unlock(&mutex_context);
    return new_entry;
}

struct EBlockContext *findEBlockContext(const char *key)
{
    struct EBlockContext *entry;
    unsigned long hash = full_name_hash(key, strlen(key));
    mutex_lock(&mutex_context);
    hash_for_each_possible(BlockContexts, entry, hash_node, hash) {
        if (strcmp(entry->name, key) == 0)
            return entry;
    }
    mutex_unlock(&mutex_context); // 释放锁
    return NULL;
}

void removeEBlockContext(const char *key)
{
    struct EBlockContext *entry = findEBlockContext(key);
    if (entry) {
        hash_del(&entry->hash_node);
        kfree(entry);
    }
}

void clearEBlockContexts(destroyBlockContext_t destroyBlockContext_cb)
{
    struct EBlockContext *entry;
    struct hlist_node *tmp;
    int bkt;
    mutex_lock(&mutex_context);; // 获取锁
    hash_for_each_safe(BlockContexts, bkt, tmp, entry, hash_node) {
        hash_del(&entry->hash_node);
        destroyBlockContext_cb(entry);
        kfree(entry);
    }
    mutex_unlock(&mutex_context);; // 释放锁

}

EXPORT_SYMBOL(newEBlockContext);
EXPORT_SYMBOL(findEBlockContext);
EXPORT_SYMBOL(removeEBlockContext);
EXPORT_SYMBOL(clearEBlockContexts);
