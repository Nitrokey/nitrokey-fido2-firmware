
# Run like:
# make callg RUN_CALLG=1 TRAVIS_COMPILER=1

ifeq ($(RUN_CALLG),1)
CFLAGS += -fdump-rtl-expand
endif

.PHONY: callg
callg: rtl2dot.py | clean $(name)
#$(src:.c=.svg)
	-rm full-graph.svg
	$(MAKE) full-graph.svg
	-xdg-open full-graph.svg

# FIXME choose best generators
GG_ALL=dot neato twopi circo fdp sfdp
GG_PREF=sfdp
%.svg: %.c.234r.expand
	python3 rtl2dot.py --local --ignore "millis|cbor|udp_|crypto_|memset|memmove|memcmp|qsort|putchar|uECC_|LOG|printf|vprintf|_check_ret" $< >$@-tmp
	for G in $(GG_ALL); do echo $${G}; $${G} -Goverlap=false -Tsvg > $@-$${G}.svg <$@-tmp ; xdg-open $@-$${G}.svg;  done
	cp $@-${GG_PREF}.svg -fv $@

full-graph.c.234r.expand:
	-rm $@
	find . -name "*.expand" -type f | grep -v -e cbor -e $@ | xargs cat > $@

rtl2dot.py:
	# https://github.com/cbdevnet/rtl2dot
	wget https://raw.githubusercontent.com/cbdevnet/rtl2dot/master/rtl2dot.py

