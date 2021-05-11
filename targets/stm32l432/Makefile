DEBUG ?= 0
HWREV ?= 3
SZ ?= $(PREFIX)arm-none-eabi-size
SOLO=nitropy fido2 util
PAGES ?= 128

# development - debug enabled, debug console, debug bootloader keys and certificates, no flash protection
# test - development minus debug console
# release - production keys and certificate, flash and debug access protection, no debug console


# RELEASE == 1 selects production public key for the bootloader, and R == 0 development one
# selects the production attestation certificate
# sets the --lock
RELEASE ?= 0

include build/common.mk
APPMAKE=build/application.mk VERSION_FULL=${VERSION_FULL}
BOOTMAKE=build/bootloader.mk VERSION_FULL=${VERSION_FULL}

KEY_PATH ?= "../../keys/attestation/device_key-dummy.hex"
KEY=$(shell cat $(KEY_PATH))
ATTEST_PATH ?= "../../keys/attestation/device_cert.der"
merge_hex_release=$(SOLO) mergehex --attestation-key "$(KEY)" --attestation-cert $(ATTEST_PATH) --lock --pages $(PAGES)

ATTEST_PATH_DEV ?= "../../keys/attestation/dev/device_cert.der"
KEY_PATH_DEV ?= "../../keys/attestation/dev/device_key.hex"
KEY_DEV=$(shell cat $(KEY_PATH_DEV))
merge_hex_dev=$(SOLO) mergehex --attestation-key "$(KEY_DEV)" --attestation-cert $(ATTEST_PATH_DEV) --lock --pages $(PAGES)

.PHONY: all all-hacker all-locked debugboot-app debugboot-boot boot-sig-checking boot-no-sig build-release-locked build-release build-release build-hacker build-debugboot clean clean2 flash flash_dfu flashboot detach cbor test

ifeq ($(RELEASE),0)
TEST_MODE_DEFINE="-DNK_TEST_MODE=1"
merge_hex=$(merge_hex_dev)
else
merge_hex=$(merge_hex_release)
endif

# The following are the main targets for reproducible builds.
# TODO: better explanation
firmware:
	$(MAKE) -f $(APPMAKE) -j8 solo.hex PREFIX=$(PREFIX) PAGES=$(PAGES) DEBUG=0 EXTRA_DEFINES='-DHWREV=$(HWREV) -DPAGES=$(PAGES) $(TEST_MODE_DEFINE)' 2>&1 | tee build.log
	test -f solo.hex || false

firmware-debug-1:
	$(MAKE) -f $(APPMAKE) -j8 solo.hex PREFIX=$(PREFIX) DEBUG=1 EXTRA_DEFINES='-DHWREV=$(HWREV) -DPAGES=$(PAGES) $(TEST_MODE_DEFINE)' 2>&1 | tee build.log
	test -f solo.hex || false

.PHONY: show-warnings
show-warnings:
	@echo
	grep "warning:" build.log | sort -u
	@echo
	grep -F '#warning' build-*.log | grep -v "warning:" | sort -u
	@echo

firmware-debug-2:
	$(MAKE) -f $(APPMAKE) -j8 solo.hex PREFIX=$(PREFIX) DEBUG=2 EXTRA_DEFINES='-DHWREV=$(HWREV) -DPAGES=$(PAGES) $(TEST_MODE_DEFINE)'

bootloader-nonverifying:
	$(MAKE) -f $(BOOTMAKE) -j8 bootloader.hex PREFIX=$(PREFIX) PAGES=$(PAGES) EXTRA_DEFINES='-DSOLO_HACKER $(TEST_MODE_DEFINE) -DPAGES=$(PAGES)' DEBUG=0

bootloader-verifying:
	$(MAKE) -f $(BOOTMAKE) -j8 bootloader.hex PREFIX=$(PREFIX) PAGES=$(PAGES) EXTRA_DEFINES='$(TEST_MODE_DEFINE) -DPAGES=$(PAGES)' DEBUG=0 2>&1 | tee build-bootloader.log
	test -f bootloader.hex || false


full-clean: clean2

.PHONY: debug-release-buildv
ver=$(VERSION_FULL)
signamedev=release/nitrokey-fido2-firmware-dev-$(ver)-$(PAGES)
debug-release-buildv: | full-clean cbor bootloader-verifying clean firmware-debug-1 show-warnings
	mkdir -p release/
	$(VENV) $(merge_hex) solo.hex bootloader.hex all.hex
	cp all.hex $(signamedev)-all-to_flash.hex
	cp solo.hex $(signamedev)-app-to_sign.hex
	sha256sum  $(signamedev)-app-to_sign.hex $(signamedev)-all-to_flash.hex *.hex | sort
	$(SOLO) sign ../../keys/bootloader-dev.pem solo.hex $(signamedev).json
	sha256sum $(signamedev).json > $(signamedev).json.sha2
	@echo "done"

.PHONY: release-buildv
signame1=release/nitrokey-fido2-firmware-$(ver)-$(PAGES)
signame=$(signame1).json
ifeq ($(RELEASE),0)
signame1=release/nitrokey-fido2-firmware-$(ver)-test-$(PAGES)
endif

release-buildv: | full-clean cbor bootloader-verifying clean firmware show-warnings
	mkdir -p release/
	-rm release/*
	$(VENV) $(merge_hex) solo.hex bootloader.hex all.hex
	cp all.hex $(signame1)-all-to_flash.hex
	cp solo.hex $(signame1)-app-to_sign.hex
	sha256sum  $(signame1)-app-to_sign.hex $(signame1)-all-to_flash.hex *.hex | sort
	ls -la release/
	@echo "done"

.PHONY: test-signed-flash
test-signed-flash: | flash
	-$(SOLO) program aux enter-bootloader
	$(SOLO) program bootloader $(signame)

.PHONY: debug-release-build
debug-release-build: | full-clean cbor bootloader-nonverifying clean firmware
	$(VENV) $(merge_hex) solo.hex bootloader.hex all.hex
	echo "done"

.PHONY: debug-release-archive
ARCH_OUT=nitrokey-fido2-`git describe --long`.zip
debug-release-archive: solo.hex all.hex bootloader.hex
	7za a $(ARCH_OUT) solo.hex all.hex bootloader.hex
	gpg2 --detach-sign $(ARCH_OUT)
	ls -hl $(ARCH_OUT)*

# The older targets, may be re-organised
all:
	$(MAKE) -f $(APPMAKE) -j8 solo.hex PREFIX=$(PREFIX) DEBUG=$(DEBUG) EXTRA_DEFINES='-DFLASH_ROP=1'

all-hacker:
	$(MAKE) -f $(APPMAKE) -j8 solo.hex PREFIX=$(PREFIX) DEBUG=$(DEBUG) EXTRA_DEFINES='-DSOLO_HACKER -DFLASH_ROP=0'

all-locked:
	$(MAKE) -f $(APPMAKE) -j8 solo.hex PREFIX=$(PREFIX) EXTRA_DEFINES='-DFLASH_ROP=2'

debugboot-app:
	$(MAKE) -f $(APPMAKE) -j8 solo.hex DEBUG=2 PREFIX=$(PREFIX) \
	LDSCRIPT=linker/stm32l4xx_extra.ld EXTRA_DEFINES='-DAPPLICATION_START_PAGE=16 -DSOLO_HACKER -DHWREV=$(HWREV) $(TEST_MODE_DEFINE)'

debugboot-boot:
	$(MAKE) -f $(BOOTMAKE) -j8 bootloader.hex PREFIX=$(PREFIX) DEBUG=1 \
	LDSCRIPT=linker/bootloader_stm32l4xx_extra.ld EXTRA_DEFINES='-DAPPLICATION_START_PAGE=16 -DSOLO_HACKER $(TEST_MODE_DEFINE)'

boot-sig-checking:
	$(MAKE) -f $(BOOTMAKE) -j8 bootloader.hex PREFIX=$(PREFIX) DEBUG=0

boot-no-sig:
	$(MAKE) -f $(BOOTMAKE) -j8 bootloader.hex PREFIX=$(PREFIX) EXTRA_DEFINES='-DSOLO_HACKER' DEBUG=0

build-release-locked: cbor clean2 boot-sig-checking clean all-locked
	$(VENV) $(merge_hex) solo.hex bootloader.hex all.hex

build-release: cbor clean2 boot-sig-checking clean all
	$(VENV) $(merge_hex) solo.hex bootloader.hex all.hex

build-hacker: cbor clean2 boot-no-sig clean all-hacker
	$(VENV) $(merge_hex) solo.hex bootloader.hex all.hex

build-debugboot: | clean2 debugboot-boot clean debugboot-app
	$(VENV) $(merge_hex) solo.hex bootloader.hex all.hex

clean:
	$(MAKE) -f $(APPMAKE) clean
	$(MAKE) -f $(BOOTMAKE) clean
clean2:
	rm -f solo.hex solo.map bootloader.hex bootloader.map all.hex
	$(MAKE) -f $(APPMAKE) clean
	$(MAKE) -f $(BOOTMAKE) clean

FW=all.hex
flash: solo.hex bootloader.hex all.hex
#	$(VENV) $(merge_hex) solo.hex bootloader.hex all.hex
	STM32_Programmer_CLI -c port=SWD -halt -e all --readunprotect
	STM32_Programmer_CLI -c port=SWD -halt  -d $(FW) -rst

flash_dfu: solo.hex bootloader.hex
	$(VENV) $(merge_hex) solo.hex bootloader.hex all.hex
	# STM32_Programmer_CLI -c port=usb1 -halt -e all --readunprotect
	STM32_Programmer_CLI -c port=usb1 -halt -rdu  -d all.hex

flashboot: bootloader.hex
	STM32_Programmer_CLI -c port=SWD -halt -e all --readunprotect
	STM32_Programmer_CLI -c port=SWD -halt  -d bootloader.hex -rst

flash-firmware:
	$(SZ) -A solo.elf
	-$(SOLO) program aux enter-bootloader
	$(SOLO) program bootloader $(signamedev).json

flash-firmware-final:
	arm-none-eabi-size -A solo.elf
	-$(SOLO) program aux enter-bootloader
	$(SOLO) program bootloader $(signamedev)

# tell ST DFU to enter application
detach:
	STM32_Programmer_CLI -c port=usb1 -ob nBOOT0=1

bootloader.hex:
	echo "You need to build the bootloader first."

solo.hex:
	echo "You need to build the application first."

cbor:
	$(MAKE) -f $(APPMAKE) -j8 cbor

test:
	$(MAKE) debug-release-buildv
	$(MAKE) release-buildv RELEASE=1

.PHONY: run-debug
run-debug:
	openocd -f interface/stlink-v2.cfg  -f target/stm32l4x.cfg

.PHONY: run-reset
run-reset:
	STM32_Programmer_CLI -c port=SWD -rst

.PHONY: run-monitor
run-monitor:
	nitropy fido2 util monitor /dev/nitrokey-fido2-serial
