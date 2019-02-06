
# Disable ASAN, while running on CI
ifeq ($(TRAVIS_COMPILER),)
RUN_ASAN := 1
endif

ifeq ($(RUN_ASAN),1)
# Enable / disable ASAN
CFLAGS_ASAN= -fsanitize=address -O1 -g -fno-omit-frame-pointer
LDFLAGS_ASAN = -lasan

#CFLAGS_ASAN= -fsanitize=undefined -O1 -g -fno-omit-frame-pointer
#LDFLAGS_ASAN = -lubsan
#
#CFLAGS_ASAN= -fsanitize=leak -O1 -g -fno-omit-frame-pointer
#LDFLAGS_ASAN = -llsan

# Tested on clang only
#CFLAGS_ASAN= -fsanitize=memory -fsanitize-memory-track-origins  -O1 -g -fno-omit-frame-pointer -fPIE -pie
#LDFLAGS_ASAN = -lmsan

CFLAGS += $(CFLAGS_ASAN)
LDFLAGS += $(LDFLAGS_ASAN)
endif

COV_FLAGS = --coverage
ifeq ($(RUN_COV),1)
CFLAGS += $(COV_FLAGS)
LDFLAGS += $(COV_FLAGS)
endif