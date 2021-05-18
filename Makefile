include fido2/version.mk

#define uECC_arch_other 0
#define uECC_x86        1
#define uECC_x86_64     2
#define uECC_arm        3
#define uECC_arm_thumb  4
#define uECC_arm_thumb2 5
#define uECC_arm64      6
#define uECC_avr        7
ecc_platform=2

src = pc/device.c pc/main.c

obj = $(src:.c=.o)

LIBCBOR = tinycbor/lib/libtinycbor.a
LIBSOLO = fido2/libsolo.a

ifeq ($(shell uname -s),Darwin)
  export LDFLAGS = -Wl,-dead_strip
else
  export LDFLAGS = -Wl,--gc-sections
endif
LDFLAGS += $(LIBSOLO) $(LIBCBOR)


CFLAGS = -O2 -fdata-sections -ffunction-sections -g
ECC_CFLAGS = -O2 -fdata-sections -ffunction-sections -DuECC_PLATFORM=$(ecc_platform)

INCLUDES =  -I../ -I./fido2/ -I./pc -I../pc -I./tinycbor/src

CFLAGS += $(INCLUDES)
CFLAGS += -DAES256=1  -DSOLO_EXPERIMENTAL=1 -DDEBUG_LEVEL=1

name = main

.PHONY: all $(LIBCBOR) $(LIBSOLO) black blackcheck cppcheck wink fido2-test clean full-clean ci test clean version
all: main

tinycbor/Makefile crypto/tiny-AES-c/aes.c:
	git submodule update --init

.PHONY: cbor
cbor: $(LIBCBOR)

$(LIBCBOR):
	cd tinycbor/ && $(MAKE)  LDFLAGS='' -j8

$(LIBSOLO):
	cd fido2/ && $(MAKE) CFLAGS="$(CFLAGS)" ECC_CFLAGS="$(ECC_CFLAGS)" APP_CONFIG=app.h -j8

version:
	@git describe

test: venv
	$(MAKE) clean
	-$(MAKE) -C . main
	$(MAKE) clean
	$(MAKE) -C ./targets/stm32l432 test PREFIX=$(PREFIX) "VENV=$(VENV)" VERSION_FULL=${SOLO_VERSION_FULL}
	$(MAKE) clean
	-$(MAKE) cppcheck

$(name): $(obj) $(LIBCBOR) $(LIBSOLO)
	$(CC) $(LDFLAGS) -o $@ $(obj) $(LDFLAGS)

venv:
	python3 -m venv venv
	venv/bin/pip -q install --upgrade pip
	venv/bin/pip -q install --upgrade -r tools/requirements.txt
	venv/bin/pip -q install --upgrade black

# selectively reformat our own code
black: venv
	venv/bin/black --skip-string-normalization --check tools/

wink: venv
	venv/bin/solo key wink

fido2-test: venv
	venv/bin/python tools/ctap_test.py

update:
	git fetch --tags
	git checkout master
	git rebase origin/master
	git submodule update --init --recursive


CPPCHECK_FLAGS=--quiet --error-exitcode=2

cppcheck:
	cppcheck $(CPPCHECK_FLAGS) crypto/aes-gcm
	cppcheck $(CPPCHECK_FLAGS) crypto/sha256
	cppcheck $(CPPCHECK_FLAGS) fido2
	cppcheck $(CPPCHECK_FLAGS) pc

clean:
	rm -f *.o main.exe main $(obj)
	for f in crypto/tiny-AES-c/Makefile tinycbor/Makefile ; do \
	    if [ -f "$$f" ]; then \
	    	(cd `dirname $$f` ; git checkout -- .) ;\
	    	(cd `dirname $$f` ; make clean) ;\
	    fi ;\
	done
	cd fido2 && $(MAKE) clean

full-clean: clean
	rm -rf venv

ci:
	git submodule update --init --recursive
	$(MAKE) docker-build-toolchain
	$(MAKE) docker-build-all

.PHONY: simulation
simulation: main
	-rm -v authenticator_state*.bin resident_keys.bin
	while true; do ./main; sleep 0.1; done;

include docker_build.mk
