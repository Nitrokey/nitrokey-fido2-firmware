
.PHONY: $(TARGET).buildinfo
$(TARGET).buildinfo:
	date > $@
	$(CC) --version >> $@
	-git describe --long >> $@
	-git describe --long --all >> $@
	@echo CC=$(CC) >> $@
	@echo CFLAGS=$(CFLAGS) >> $@
	@echo >> $@
	@echo LDFLAGS=$(LDFLAGS) >> $@
	@echo AOBJARM=$(AOBJARM) >> $@
	@echo THUMB=$(THUMB) >> $@
	@echo ALL_CFLAGS=$(ALL_CFLAGS) >> $@
	@echo AOBJ=$(AOBJ) >> $@
	@echo COBJARM=$(COBJARM) >> $@
	@echo COBJ=$(COBJ) >> $@
	@echo CPPOBJ=$(CPPOBJ) >> $@
	@echo CPPOBJARM=$(CPPOBJARM) >> $@
	@echo ECC_CFLAGS=$(ECC_CFLAGS) >> $@
	@echo "Writing build information to $@"
