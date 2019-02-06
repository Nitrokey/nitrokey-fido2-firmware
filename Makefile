#!/usr/bin/make -f

#define uECC_arch_other 0
#define uECC_x86        1
#define uECC_x86_64     2
#define uECC_arm        3
#define uECC_arm_thumb  4
#define uECC_arm_thumb2 5
#define uECC_arm64      6
#define uECC_avr        7

ecc_platform=2

src = $(wildcard pc/*.c) $(wildcard fido2/*.c) $(wildcard crypto/sha256/*.c) crypto/tiny-AES-c/aes.c
obj = $(src:.c=.o) uECC.o

LIBCBOR = tinycbor/lib/libtinycbor.a

ifeq ($(shell uname -s),Darwin)
  export LDFLAGS = -Wl,-dead_strip
else
  export LDFLAGS = -Wl,--gc-sections
endif
LDFLAGS += $(LIBCBOR)
CFLAGS = -fdata-sections -ffunction-sections

INCLUDES = -I./tinycbor/src -I./crypto/sha256 -I./crypto/micro-ecc/ -Icrypto/tiny-AES-c/ -I./fido2/ -I./pc -I./fido2/extensions

CFLAGS += $(INCLUDES)
# for crypto/tiny-AES-c
CFLAGS += -DAES256=1 -DAPP_CONFIG=\"app.h\"

name = solo-simulation-main

.PHONY: all
all: $(name)

include Makefile.obsolete
include callgraph.mk
include coverage.mk
include code_check.mk
include sanitazers.mk
include simulation_test.mk

tinycbor/Makefile crypto/tiny-AES-c/aes.c:
	git submodule update --init

.PHONY: cbor
cbor: $(LIBCBOR)

$(LIBCBOR): tinycbor/Makefile
	cd tinycbor/ && $(MAKE) clean && $(MAKE) "CFLAGS_EXTRA=$(CFLAGS_ASAN)" "LDFLAGS_EXTRA=$(LDFLAGS_ASAN)"
#	cd tinycbor/ && $(MAKE) clean && $(MAKE)

VERSION=$(shell git describe --always --long), build time: $(shell date)
.PHONY: update_version
update_version:
	@echo Update version string to \"${VERSION}\"
	sed -e "s/@VERSION_STRING@/${VERSION}/g" fido2/version.h.in > fido2/version.h


$(name): update_version $(obj) $(LIBCBOR)
	$(CC) $(LDFLAGS) -o $@ $(obj) $(LDFLAGS)
	@echo
	@echo Output binary:
	@ls -lh $(name)
	@readlink -f $(name)

uECC.o: ./crypto/micro-ecc/uECC.c
	$(CC) -c -o $@ $^ -O2 -fdata-sections -ffunction-sections -DuECC_PLATFORM=$(ecc_platform) -I./crypto/micro-ecc/


env3:
	python3 -m venv env3
	env3/bin/pip install --upgrade -r tools/requirements.txt
	env3/bin/pip install --upgrade black

# selectively reformat our own code
black: env3
	env3/bin/black --skip-string-normalization tools/

black_test: env3
	env3/bin/black --skip-string-normalization --check tools/

wink3: env3
	env3/bin/python tools/solotool.py solo --wink

fido2-test: env3
	# tests real device
	env3/bin/python3 tools/ctap_test.py

test: $(name) cppcheck

.PHONY: clean
clean:
	$(MAKE) -C tinycbor clean
	rm -f *.o $(name).exe $(name) $(obj)


.PHONY: clean_all
clean_all: clean clean_subrepo
	-rm -rf env2 env3 env3_sim
	-rm -v ${CLEAN_ADDITIONAL}

.PHONY: clean_subrepo
clean_subrepo:
	for f in crypto/tiny-AES-c/Makefile tinycbor/Makefile ; do \
	    if [ -f "$$f" ]; then \
	    	(cd `dirname $$f` ; git checkout -- .) ;\
	    fi ;\
	done

.PHONY: info
info:
	@echo DEBUG: $(DEBUG)
	@echo Sources: $(src)
	@echo Objects: $(obj)
	@echo CFLAGS: $(CFLAGS)
	@echo LDFLAGS: $(LDFLAGS)
	@echo Commands: `egrep '^\w+:' Makefile | awk '{print $$1}' | sort | tr -d '\r\n' `
