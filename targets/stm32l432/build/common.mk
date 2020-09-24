include ../../fido2/version.mk

CC=$(PREFIX)arm-none-eabi-gcc
CP=$(PREFIX)arm-none-eabi-objcopy
SZ=$(PREFIX)arm-none-eabi-size
AR=$(PREFIX)arm-none-eabi-ar
AS=$(PREFIX)arm-none-eabi-as

DRIVER_LIBS := lib/stm32l4xx_hal_pcd.c lib/stm32l4xx_hal_pcd_ex.c lib/stm32l4xx_ll_gpio.c  \
       lib/stm32l4xx_ll_rcc.c lib/stm32l4xx_ll_rng.c lib/stm32l4xx_ll_tim.c  \
	   lib/stm32l4xx_ll_usb.c lib/stm32l4xx_ll_utils.c lib/stm32l4xx_ll_pwr.c \
	   lib/stm32l4xx_ll_usart.c lib/stm32l4xx_ll_spi.c lib/stm32l4xx_ll_exti.c

USB_LIB := lib/usbd/usbd_cdc.c lib/usbd/usbd_cdc_if.c lib/usbd/usbd_composite.c \
	   lib/usbd/usbd_conf.c lib/usbd/usbd_core.c lib/usbd/usbd_ioreq.c \
       lib/usbd/usbd_ctlreq.c lib/usbd/usbd_desc.c lib/usbd/usbd_hid.c \
	   lib/usbd/usbd_ccid.c

VERSION:=$(shell git describe --abbrev=0 )
VERSION_FULL_RAW:=$(shell git describe)
VERSION_FULL:=$(shell python3 -c 'print("$(VERSION_FULL_RAW)".strip(".nitrokey")) if ".nitrokey" in "$(VERSION_FULL_RAW)" else exit(1)')
VERSION_FULL:=$(if $(VERSION_FULL),$(VERSION_FULL),$(error Invalid version tag - no '.nitrokey' suffix))

VERSION_MAJ:=$(shell python3 -c 'print("$(VERSION)".split(".")[0])')
VERSION_MIN:=$(shell python3 -c 'print("$(VERSION)".split(".")[1])')
VERSION_PAT:=$(shell python3 -c 'print("$(VERSION)".split(".")[2])')

#VERSION_FULL?=$(SOLO_VERSION_FULL)
#VERSION:=$(SOLO_VERSION)
#VERSION_MAJ:=$(SOLO_VERSION_MAJ)
#VERSION_MIN:=$(SOLO_VERSION_MIN)
#VERSION_PAT:=$(SOLO_VERSION_PAT)

VERSION_FLAGS= -DSOLO_VERSION_MAJ=$(VERSION_MAJ) -DSOLO_VERSION_MIN=$(VERSION_MIN) \
	-DSOLO_VERSION_PATCH=$(VERSION_PAT) -DSOLO_VERSION=\"$(VERSION_FULL)\"

.PHONY: version-all
version-all:
	@echo $(VERSION_FULL_RAW)
	@echo $(SOLO_VERSION_FULL)
	@echo $(SOLO_VERSION_MAJ)
	@echo $(SOLO_VERSION_MIN)
	@echo $(SOLO_VERSION_PAT)

%.o: %.s
	@echo "*** $<"
	$(AS) -o $@ $^
