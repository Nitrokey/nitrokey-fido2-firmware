
# Run like:
# make callg RUN_CALLG=1 RUN_ASAN=0

ifeq ($(RUN_CALLG),1)
CFLAGS += -fdump-rtl-expand
endif

.PHONY: callg
CG_WD=reports/CG
CG_OUT=$(CG_WD)/call-graph.svg
callg: $(CG_WD)/rtl2dot.py | clean $(name)
#$(src:.c=.svg)
	-rm $(CG_OUT)
	$(MAKE) $(CG_OUT)
	-xdg-open $(CG_OUT)

# FIXME choose best generators
GG_ALL=dot neato twopi circo fdp sfdp
GG_PREF=sfdp
%.svg: %.c.234r.expand
	python3 rtl2dot.py --local --ignore "millis|cbor|udp_|crypto_|memset|memmove|memcmp|qsort|putchar|uECC_|LOG|printf|vprintf|_check_ret" $< >$@-tmp
	for G in $(GG_ALL); do echo $${G}; $${G} -Goverlap=false -Tsvg > $@-$${G}.svg <$@-tmp ; xdg-open $@-$${G}.svg;  done
	cp $@-${GG_PREF}.svg -fv $@

$(CG_WD)/call-graph.c.234r.expand:
	mkdir -p $(CG_WD)
	-rm $@
	find . -name "*.expand" -type f | grep -v -e cbor -e $@ | xargs cat > $@

$(CG_WD)/rtl2dot.py:
	mkdir -p $(CG_WD)
	# https://github.com/cbdevnet/rtl2dot
	wget https://raw.githubusercontent.com/cbdevnet/rtl2dot/master/rtl2dot.py -O $@

CLEAN_ADDITIONAL += $(CG_WD)/rtl2dot.py $(src:.c=.c.234r.expand) $(src:.c=.svg) $(CG_WD)/full-graph.c.234r.expand