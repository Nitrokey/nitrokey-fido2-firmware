DEBUG := 0

venv=../../env3/bin/python3
merge_hex=$(venv) ../../tools/solotool.py mergehex

all:
	$(MAKE) -f application.mk -j8 solo.hex PREFIX=$(PREFIX) DEBUG=$(DEBUG) EXTRA_DEFINES='-DFLASH_ROP=1'

all-hacker:
	$(MAKE) -f application.mk -j8 solo.hex PREFIX=$(PREFIX) DEBUG=$(DEBUG) EXTRA_DEFINES='-DSOLO_HACKER -DFLASH_ROP=0'

all-locked:
	$(MAKE) -f application.mk -j8 solo.hex PREFIX=$(PREFIX) EXTRA_DEFINES='-DFLASH_ROP=2'

debugboot-app:
	$(MAKE) -f application.mk -j8 solo.hex DEBUG=2 PREFIX=$(PREFIX)\
	LDSCRIPT=linker/stm32l4xx_extra.ld EXTRA_DEFINES='-DAPPLICATION_START_PAGE=16 -DSOLO_HACKER'

debugboot-boot:
	$(MAKE) -f bootloader.mk -j8 bootloader.hex PREFIX=$(PREFIX) DEBUG=1 \
	LDSCRIPT=linker/bootloader_stm32l4xx_extra.ld EXTRA_DEFINES='-DAPPLICATION_START_PAGE=16 -DSOLO_HACKER'

boot-sig-checking:
	$(MAKE) -f bootloader.mk -j8 bootloader.hex PREFIX=$(PREFIX) DEBUG=0

boot-no-sig:
	$(MAKE) -f bootloader.mk -j8 bootloader.hex PREFIX=$(PREFIX) EXTRA_DEFINES='-DSOLO_HACKER' DEBUG=0

build-release-locked: clean2 boot-sig-checking clean all-locked
	$(merge_hex) solo.hex bootloader.hex all.hex
	rm -f solo.hex bootloader.hex  # don't program solo.hex ...

build-release: clean2 boot-sig-checking clean all
	$(merge_hex) solo.hex bootloader.hex all.hex

VERSION=$(shell git describe --always --long), build time: $(shell date)
.PHONY: update_version
update_version:
	@echo Update version string to \"${VERSION}\"
	sed -e "s/@VERSION_STRING@/${VERSION}/g" ../../fido2/version.h.in > ../../fido2/version.h

build-hacker:
	$(MAKE) update_version
	$(MAKE) clean2
	$(MAKE) boot-no-sig
	$(MAKE) clean
	$(MAKE) all-hacker
	$(merge_hex) solo.hex bootloader.hex all.hex
	@ls -lh solo.hex bootloader.hex all.hex
	@readlink -f all.hex

clean:
	$(MAKE) -f application.mk clean
	$(MAKE) -f bootloader.mk clean
clean2:
	rm -f solo.hex bootloader.hex all.hex
	$(MAKE) -f application.mk clean
#	$(MAKE) -f application.mk cbor_clean
	$(MAKE) -f bootloader.mk clean


flash: solo.hex bootloader.hex
	$(merge_hex) solo.hex bootloader.hex all.hex
	STM32_Programmer_CLI -c port=SWD -halt -e all --readunprotect
	STM32_Programmer_CLI -c port=SWD -halt  -d all.hex -rst

flash_dfu: solo.hex bootloader.hex
	$(merge_hex) solo.hex bootloader.hex all.hex
	# STM32_Programmer_CLI -c port=usb1 -halt -e all --readunprotect
	STM32_Programmer_CLI -c port=usb1 -halt -rdu  -d all.hex

flashboot: solo.hex bootloader.hex
	$(merge_hex) solo.hex bootloader.hex all.hex
	STM32_Programmer_CLI -c port=SWD -halt -e all --readunprotect
	STM32_Programmer_CLI -c port=SWD -halt  -d bootloader.hex -rst

# tell ST DFU to enter application
detach:
	STM32_Programmer_CLI -c port=usb1 -ob nBOOT0=1

bootloader.hex:
	echo "You need to build the bootloader first."

solo.hex:
	echo "You need to build the application first."

cbor:
	$(MAKE) -f application.mk -j8 cbor


.PHONY: scan_build
ARM_GCC=$(shell which arm-none-eabi-gcc)
# --status-bugs
SBO=--use-cc=$(ARM_GCC) --analyzer-target=arm-none-eabi -o reports/scan-build --show-description
scan_build: clean2 cbor
# 	boot-no-sig clean all-hacker
	scan-build $(SBO) $(MAKE) boot-no-sig
	make clean2
	scan-build $(SBO) $(MAKE) all-hacker
	xdg-open reports/scan-build

.PHONY: info
info:
	@echo DEBUG: $(DEBUG)
	@echo Commands: `egrep '^\w+:' Makefile | awk '{print $$1}' | sort | tr -d '\r\n' `
	$(MAKE) info -f bootloader.mk
	$(MAKE) info -f application.mk
