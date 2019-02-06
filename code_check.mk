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
