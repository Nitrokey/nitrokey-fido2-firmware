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
CFLAGS = -O2 -fdata-sections -ffunction-sections

INCLUDES = -I./tinycbor/src -I./crypto/sha256 -I./crypto/micro-ecc/ -Icrypto/tiny-AES-c/ -I./fido2/ -I./pc -I./fido2/extensions

CFLAGS += $(INCLUDES)
# for crypto/tiny-AES-c
CFLAGS += -DAES256=1 -DAPP_CONFIG=\"app.h\"

name = main

.PHONY: all
all: main

include Makefile.obsolete

tinycbor/Makefile crypto/tiny-AES-c/aes.c:
	git submodule update --init

.PHONY: cbor
cbor: $(LIBCBOR)

$(LIBCBOR): tinycbor/Makefile
	cd tinycbor/ && $(MAKE) clean && $(MAKE) -j8

$(name): $(obj) $(LIBCBOR)
	$(CC) $(LDFLAGS) -o $@ $(obj) $(LDFLAGS)

uECC.o: ./crypto/micro-ecc/uECC.c
	$(CC) -c -o $@ $^ -O2 -fdata-sections -ffunction-sections -DuECC_PLATFORM=$(ecc_platform) -I./crypto/micro-ecc/

env2:
	virtualenv --python=python2.7 env2
	env2/bin/pip install --upgrade -r tools/requirements.txt

env3:
	python3 -m venv env3
	env3/bin/pip install --upgrade -r tools/requirements.txt
	env3/bin/pip install --upgrade black

# selectively reformat our own code
black: env3
	env3/bin/black --skip-string-normalization tools/

black_test: env3
	env3/bin/black --skip-string-normalization --check tools/


wink2: env2
	env2/bin/python tools/solotool.py solo --wink

wink3: env3
	env3/bin/python tools/solotool.py solo --wink

fido2-test: env3
	env3/bin/python tools/ctap_test.py

CPPCHECK_FLAGS := --quiet --error-exitcode=2
.PHONY: cppcheck
cppcheck:
	cppcheck --version
	cppcheck $(CPPCHECK_FLAGS) crypto/aes-gcm
	cppcheck $(CPPCHECK_FLAGS) crypto/sha256
	cppcheck $(CPPCHECK_FLAGS) fido2
	cppcheck $(CPPCHECK_FLAGS) pc
	cppcheck $(CPPCHECK_FLAGS) targets/stm32l432 --force
	cppcheck $(CPPCHECK_FLAGS) tinycbor --force
	cppcheck $(CPPCHECK_FLAGS) crypto/micro-ecc/ --force

test: main cppcheck black_test

.PHONY: clean
clean:
	rm -f *.o main.exe main $(obj)
	rm -rf env2 env3
	for f in crypto/tiny-AES-c/Makefile tinycbor/Makefile ; do \
	    if [ -f "$$f" ]; then \
	    	(cd `dirname $$f` ; git checkout -- .) ;\
	    fi ;\
	done
