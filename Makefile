#obj-m := nulldev.o
#obj-m := lightnvm.o
#obj-m := ramdisk.o
obj-m := rufs.o
rufs-objs := rufslib.o ramufs.o

CURRENT_PATH := $(shell pwd)
LINUX_KERNEL_PATH := /root/linufs

all:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
