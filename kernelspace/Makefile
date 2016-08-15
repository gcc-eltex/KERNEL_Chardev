KERNELDIR:=/usr/src/linux-headers-$(shell uname -r)
PWD := $(shell pwd)
ccflags-y := -std=gnu99
obj-m += chdev.o

all:
	sudo $(MAKE) -C $(KERNELDIR) M=$(PWD) modules

install:
	sudo insmod ./chdev.ko

delete:
	sudo rmmod chdev

clean:
	rm -rf *.o *.ko *.order *.symvers *.mod.c .chdev.* .tmp_versions

log:
	sudo vim /var/log/syslog

lclean:
	>/var/log/syslog
