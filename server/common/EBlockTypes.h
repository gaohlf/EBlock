//
// Created by 20075 on 2024/7/3.
//

#ifndef EBLOCK_EBLOCKTYPES_H
#define EBLOCK_EBLOCKTYPES_H
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#ifdef __linux__
#include <sys/ioctl.h>
#endif
#include <errno.h>

typedef uint8_t u8;
typedef uint64_t u64;

#endif //EBLOCK_EBLOCKTYPES_H
