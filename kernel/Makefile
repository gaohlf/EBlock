obj-m += EblockIO.o
EblockIO-objs := EbSys.o Eblock.o \
ECreateBlkDev.o EBlockContext.o	\
EBlockRequestManager.o EBlockRequestQueue.o EBlockPendingMap.o \
EBlockCtrlDev.o EBLockGlobalCond.o EBlockStatics.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
