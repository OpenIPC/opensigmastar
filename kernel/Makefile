# Out-of-tree build:  make -C kernel KDIR=<kernel build dir> CHIPARCH=infinity2m
KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) CHIPARCH=$(CHIPARCH) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) CHIPARCH=$(CHIPARCH) clean

# Does KDIR keep the kernel ABI the relinked vendor modules were built for?
# (scripts/kabi-check.py, docs/kabi.md; exit status 1 when it does not)
kabi-check:
	python3 $(CURDIR)/../scripts/kabi-check.py --cross '$(CROSS_COMPILE)' \
		--arch $(or $(ARCH),arm) check --json $(CURDIR)/kabi/$(CHIPARCH).json \
		--kdir $(KDIR)

.PHONY: all clean kabi-check
