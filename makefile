# TYPE = test                      Enables tests
# TYPE = bench                     Enables benchmarks
# TYPE = asm                       Generates assembly
# OPTLEVEL = [0-3|g]               Sets optimization/debug
# LOGLEVEL = [1| ]                 Enables logging
# ASAN = [0|address|alignment|...] Enables specified sanitizer

MAKEFLAGS += -j$(shell nproc)

# Alias
ifdef OL
  OPTLEVEL ?= $(OL)
endif
ifdef LL
  LOGLEVEL ?= $(LL)
endif
ifdef T
  TYPE ?= $(T)
endif

OPTLEVEL ?= g

# if CC is not defined
ifeq ($(origin $(CC)),undefined)
  CC := $(if $(shell command -v clang-20),clang-20,clang)
endif
# e.g.) disable ccache
# $ make CCACHE=
CCACHE ?= $(shell command -v ccache)
ifneq ($(CCACHE),) # if CCACHE is enabled
  CC := $(CCACHE) $(CC)
endif

PROJECT_NAME := $(notdir $(shell pwd))
SRCDIR := src
INCDIR := include
BUILDDIR := .build
PREFIX ?= /usr/local

CFLAGS := -std=c23 -I$(INCDIR) -Wtautological-compare -Wsign-compare -Wextra   \
          -Wimplicit-fallthrough -Wall -O$(OPTLEVEL)
OPTFLAGS := -ffast-math -fno-finite-math-only -DNDEBUG -faddrsig -march=native \
           -mtune=native -funroll-loops -fomit-frame-pointer -fdata-sections   \
           -fforce-emit-vtables -ffunction-sections
LDFLAGS := -lm
OPTLDFLAGS := -flto=full -fwhole-program-vtables -fvirtual-function-elimination \
              -fuse-ld=lld -Wl,--gc-sections -Wl,--icf=all -s
DEBUGFLAGS := -g3
ASMFLAGS := -S -masm=intel
DEPFLAGS = -MM -MP -MF $(DEPDIR)/$*.d

# Enables macro in the source
CFLAGS += -DVERSION=\"$(shell git describe --tags --always 2>/dev/null || echo "unknown")\"
CFLAGS += -DDATE=\"$(shell date -I)\"
CFLAGS += -DLOGLEVEL=$(LOGLEVEL)

ifeq ($(TYPE),test)
  CFLAGS += -DTEST_MODE
endif

ifeq ($(TYPE),bench)
  CFLAGS += -DBENCHMARK_MODE
endif

ifdef ASAN
  CFLAGS += -fsanitize=$(ASAN)
  LDFLAGS += -fsanitize=$(ASAN)
endif

ifeq ($(OPTLEVEL),g)
  CFLAGS += $(DEBUGFLAGS)
  RUNNER ?= gdb
else ifneq ($(OPTLEVEL),0)
  CFLAGS += $(OPTFLAGS)
  LDFLAGS += $(OPTLDFLAGS)
endif

# generate output path
GITBRANCH := $(shell git branch --show-current 2>/dev/null)
SEED = $(CC)$(EXTRAFLAGS)$(CFLAGS)$(LDFLAGS)$(GITBRANCH)
HASH := $(shell echo '$(SEED)' | md5sum | cut -d' ' -f1)
OUTDIR := $(BUILDDIR)/$(HASH)
TARGETDIR := $(OUTDIR)/target
DEPDIR := $(OUTDIR)/dep

TARGET := $(TARGETDIR)/$(PROJECT_NAME)

# source files
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(TARGETDIR)/%.o,$(SRCS))
DEPS = $(patsubst $(SRCDIR)/%.c,$(DEPDIR)/%.d,$(SRCS))

-include $(DEPS)

# rules
.PHONY: run analyze clean-all clean install doc test lint fmt help release log

build: $(TARGET)

# link
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(EXTRALDFLAGS) $^ -o $@

# compile
$(TARGETDIR)/%.o: $(SRCDIR)/%.c $(DEPDIR)/%.d | $(TARGETDIR)/
	$(CC) $< -I$(INCDIR) $(CFLAGS) $(EXTRAFLAGS) -c -o $@

$(DEPDIR)/%.d: $(SRCDIR)/%.c | $(DEPDIR)/
	$(CC) $< -I$(INCDIR) $(DEPFLAGS)

%/:
	mkdir -p $@

# e.g.) run with valgrind
# make run RUNNER=valgrind
# e.g.) don't use gdb (default debug RUNNER) in debug run
# make run RUNNER=
run: $(TARGET)
	$(RUNNER) $<

# `make run-foo` is same as `make run RUNNER=foo`
run-%: $(TARGET)
	$* $<

test:
	$(MAKE) run TYPE=TEST

clean-all:
	rm -rf $(BUILDDIR)

# e.g.) remove test build for opt level 3
# make clean OPTLEVEL=3 TYPE=test
clean:
	rm -rf $(OUTDIR)

install: $(TARGET) | $(PREFIX)/
	cp $^ $(PREFIX)/bin/

uninstall:
	rm $(PREFIX)/bin/$(PROJECT_NAME)

doc: doc/Doxyfile
	doxygen $<

fmt:
	clang-format -i $(SRCS) $(INCDIR)/*.h

lint:
	clang-tidy $(SRCS) -- $(CFLAGS)

%.s: %.c
	$(CC) $< $(ASMFLAGS) $(CFLAGS) $(EXTRAFLAGS) $(DEPFLAGS) -o $@

FP ?= /dev/stdout
log:
	@echo "Compiler: $(CC)" > $(FP)
	@echo "CFLAGS: $(CFLAGS)" >> $(FP)
	@echo "LDFLAGS: $(LDFLAGS)" >> $(FP)
	@echo "TARGET: $(TARGET)" >> $(FP)
	@echo "SRCS: $(SRCS)" >> $(FP)
	@echo "OBJS: $(OBJS)" >> $(FP)
	@echo "DEPS: $(DEPS)" >> $(FP)

info: $(TARGET)
	@echo "target file size:"
	@size $(TARGET)

help:
	@echo "release build: make release"
	@echo "debug build: make"
	@echo ""
	@echo 'build files: .build/HASH/{target,dep}/*'

release:
	$(MAKE) TYPE=test OPTLEVEL=3
	$(MAKE) lint
	$(MAKE) OPTLEVEL=3
