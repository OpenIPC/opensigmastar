# Out-of-tree build:  make -C kernel KDIR=<kernel build dir> CHIPARCH=infinity2m
KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) CHIPARCH=$(CHIPARCH) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
