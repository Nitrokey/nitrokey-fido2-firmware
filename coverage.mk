# Run like:
# make coverage RUN_COV=1

.PHONY: coverage
COV_FILES=$(src:.c=.gcda) $(src:.c=.gcno) $(src:.c=.gcov)
DATE_WITH_TIME=$(shell date "+%Y%m%d-%H%M%S")
COV_TARGET_DIR=reports/coverage/${DATE_WITH_TIME}
coverage: | test_simulation $(src:.c=.gcov)
	mkdir -p ${COV_TARGET_DIR}
	lcov -b . -d . -c -o ${COV_TARGET_DIR}/simulation-run.info
	genhtml -o ${COV_TARGET_DIR}/html/ ${COV_TARGET_DIR}/simulation-run.info
	-mv -v *.gcov ${COV_TARGET_DIR}/
	xdg-open ${COV_TARGET_DIR}/html/index.html

%.gcov: %.c
	gcov -p -c -j $<

CLEAN_ADDITIONAL += $(COV_FILES)