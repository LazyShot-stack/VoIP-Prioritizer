# Makefile for VoIP Prioritizer Kernel Module
# Usage: make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

obj-m += voip_prio.o

# List all source files
voip_prio-objs := src/voip_prio.o src/voip_prio_nf.o src/voip_prio_qdisc.o src/voip_prio_sysfs.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean 