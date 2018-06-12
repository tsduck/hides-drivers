module=usb-it950x

PWD = $(shell pwd)
MACHINE = $(shell uname -m)
CURRENT = $(shell uname -r)
KERNELDIR = /lib/modules/$(CURRENT)/build
INSTALLDIR := /lib/modules/$(CURRENT)/kernel/drivers/media

$(module)-objs := \
		it950x-core.o it950x-drv.o iocontrol.o  \
		tuner.o cmd.o IT9133.o ADF4351.o \
		IT9507.o Omega.o eagleTuner.o \
		standard.o usb2impl.o modulatorUser.o user.o 

obj-m += $(module).o  

default:
	make -s -C $(KERNELDIR) SUBDIRS=$(PWD) modules

EXTRA_CFLAGS += -DEXPORT_SYMTAB -Wno-unused-value -Wno-unused-variable -Wno-unused-parameter -Wno-switch 

ifeq "$(MACHINE)" "x86_64"
EXTRA_CFLAGS += -Wno-enum-compare -Wno-int-to-pointer-cast -Wno-array-bounds
endif

install:
	install -d $(INSTALLDIR) 
	install -m 644 -c $(module).ko $(INSTALLDIR)
	/sbin/depmod -a ${KERNELRELEASE}

remove: 16.11.10.1.
	rm -f $(INSTALLDIR)/$(module).ko
	/sbin/modprobe -r $(module)

clean:
	@-rm -rf .tmp_versions *~ *.o *.ko .*.o.cmd .*.ko.cmd *.mod.c .cache.mk modules.order Module.markers Module.symvers

-include $(KERNELDIR)/Rules.make
