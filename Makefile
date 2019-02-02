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

ifeq ($(TRAVIS_COMPILER),)
# Enable / disable ASAN
CFLAGS_ASAN= -fsanitize=address -O1 -g -fno-omit-frame-pointer
LDFLAGS_ASAN = -lasan

#CFLAGS_ASAN= -fsanitize=undefined -O1 -g -fno-omit-frame-pointer
#LDFLAGS_ASAN = -lubsan
#
#CFLAGS_ASAN= -fsanitize=leak -O1 -g -fno-omit-frame-pointer
#LDFLAGS_ASAN = -llsan

# clang only
#CFLAGS_ASAN= -fsanitize=memory -fsanitize-memory-track-origins  -O1 -g -fno-omit-frame-pointer -fPIE -pie
#LDFLAGS_ASAN = -lmsan

CFLAGS += $(CFLAGS_ASAN)
LDFLAGS += $(LDFLAGS_ASAN)
endif

INCLUDES = -I./tinycbor/src -I./crypto/sha256 -I./crypto/micro-ecc/ -Icrypto/tiny-AES-c/ -I./fido2/ -I./pc -I./fido2/extensions

CFLAGS += $(INCLUDES)
# for crypto/tiny-AES-c
CFLAGS += -DAES256=1 -DAPP_CONFIG=\"app.h\"

name = solo-simulation-main

.PHONY: all
all: $(name)

include Makefile.obsolete

tinycbor/Makefile crypto/tiny-AES-c/aes.c:
	git submodule update --init

.PHONY: cbor
cbor: $(LIBCBOR)

$(LIBCBOR): tinycbor/Makefile
	cd tinycbor/ && $(MAKE) clean && $(MAKE) "CFLAGS_EXTRA=$(CFLAGS_ASAN)" "LDFLAGS_EXTRA=$(LDFLAGS_ASAN)"
#	cd tinycbor/ && $(MAKE) clean && $(MAKE)

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

env3_sim:
	python3 -m venv env3_sim
	cd python-fido2 && ../env3_sim/bin/python3 setup.py install

test_simulation: env3_sim $(name)
	$(CC) --version
	./env3_sim/bin/python3 --version
	-killall $(name)
	./$(name) &
	./env3_sim/bin/python3 tools/ctap_test.py
	./env3_sim/bin/python3 python-fido2/examples/credential.py
	./env3_sim/bin/python3 python-fido2/examples/get_info.py
	./env3_sim/bin/python3 python-fido2/examples/multi_device.py
	killall $(name)
	@echo "!!! All tests returned non-error code"

fido2-test: env3
	# tests real device
	env3/bin/python3 tools/ctap_test.py

CPPCHECK_FLAGS := --quiet --error-exitcode=2
.PHONY: cppcheck
cppcheck:
	cppcheck --version
	cppcheck $(CPPCHECK_FLAGS) crypto/aes-gcm
	cppcheck $(CPPCHECK_FLAGS) crypto/sha256
	cppcheck $(CPPCHECK_FLAGS) fido2
	cppcheck $(CPPCHECK_FLAGS) pc
	cppcheck $(CPPCHECK_FLAGS) targets/stm32l432 --force
	-cppcheck $(CPPCHECK_FLAGS) tinycbor --force
	cppcheck $(CPPCHECK_FLAGS) crypto/micro-ecc/ --force

.PHONY: clang_check
clang_check:
	clang-check --version
	find . -name '*.c' -o -name '*.h' | xargs clang-check -p .

.PHONY: clang_tidy
clang_tidy:
	clang-tidy --version
	find pc -name '*.c' -o -name '*.h' | xargs -I '{}'	clang-tidy '{}' -- ${CFLAGS}
	find fido2 -name '*.c' -o -name '*.h' | xargs -I '{}'	clang-tidy '{}' -- ${CFLAGS}
	find targets/stm32l432  -name '*.c' -o -name '*.h' | xargs -I '{}'	clang-tidy '{}' -- ${CFLAGS}

.PHONY: scan_build
CHECKERS=-enable-checker security -enable-checker alpha  -enable-checker nullability -enable-checker valist
scan_build: clean
	mkdir -p reports/scan-build
	clang-7 --version
	-scan-build -o reports/scan-build --show-description --status-bugs $(CHECKERS) $(MAKE)
	xdg-open reports/scan-build/ &

.PHONY: scan_build_arm
scan_build_arm: clean
	$(MAKE) scan_build -C targets/stm32l432/

test: $(name) cppcheck

.PHONY: clean
clean:
	make -C tinycbor clean
	rm -f *.o $(name).exe $(name) $(obj)


.PHONY: clean_all
clean_all: clean clean_subrepo
	rm -rf env2 env3 env3_sim

.PHONY: clean_subrepo
clean_subrepo:
	for f in crypto/tiny-AES-c/Makefile tinycbor/Makefile ; do \
	    if [ -f "$$f" ]; then \
	    	(cd `dirname $$f` ; git checkout -- .) ;\
	    fi ;\
	done

.PHONY: info
info:
	@echo Sources: $(src)
	@echo Objects: $(obj)
	@echo CFLAGS: $(CFLAGS)
	@echo LDFLAGS: $(LDFLAGS)
	@echo Commands: `egrep '^\w+:' Makefile | awk '{print $$1}' | sort | tr -d '\r\n' `
