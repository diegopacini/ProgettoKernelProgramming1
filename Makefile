KERNEL_DIR ?= /lib/modules/`uname -r`/build

obj-m = my_mod.o

all:
	make -C $(KERNEL_DIR) M=`pwd` modules

clean:
	make -C $(KERNEL_DIR) M=`pwd` clean

