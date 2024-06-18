#ifndef EBLOCKREQUESTS_H
#define EBLOCKREQUESTS_H

#define SECTOR_SIZE 512L
#define BLOCK_CONTEXT_NAME_SIZE 39
#define EBLOCK_CURRENT_VERSION  1
#define EBLOCK_MAGIC            123
//单次最大获取的请求数 加上EBRequests正好一个2K 超过2K后会使得 EBRequests过大不适宜在站上申请
#define MAX_SWAP_REQUESTS_ONCE  31


#ifdef __cplusplus
extern "C" {
#endif

/***
 * 用户态和内核态交互获取请求的数据结构
 * 目前总共 64字节
 * 每个IO会产生一个数据头
 * **********************/
struct __attribute__((packed)) EBRequest
{
    char devName[BLOCK_CONTEXT_NAME_SIZE];//对应的块设备名称
    u8    isWrite; //读写标志
    u64   length;   //请求的字节数
    u64   off;      //相对于块的偏移量 单位为字节
    u64   kernelID; //相关的内核模块的唯一标识，可以根据这个值查到对应request 地址，这个是read/write的唯一标识
};

//一次数据传递 不超过一个page
struct __attribute__((packed)) EBRequests
{
    u64 requestNum;//本次取到的request数量
    struct EBRequest requests[MAX_SWAP_REQUESTS_ONCE]; //request头列表
};

#ifdef __cplusplus
}
#endif


#endif
