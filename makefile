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

CC1 := $(shell command -v clang-21)
CC2 := $(shell command -v clang-20)
CC3 := $(shell command -v clang-19)

# if CC is not defined
ifeq ($(origin $(CC)),undefined)
  CC := $(or $(CC1),$(CC2),$(CC3),$(CC))
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

# compiler flags
CFLAGS := -std=c2y -I$(INCDIR) -Wtautological-compare -Wextra -Wall \
          -Wimplicit-fallthrough -Wbitwise-instead-of-logical -O$(OPTLEVEL) \
		  -Wconversion -Wdangling -Wdeprecated -Wdocumentation -Wmicrosoft \
		  -Wswitch-enum -Wswitch-default -Wtype-limits -Wunreachable-code-aggressive -Wpedantic \
		  -Wdocumentation-pedantic \
		  -Wno-dollar-in-identifier-extension -Wno-gnu
OPTFLAGS := -ffast-math -fno-finite-math-only -DNDEBUG -faddrsig -march=native \
           -mtune=native -funroll-loops -fomit-frame-pointer -fdata-sections   \
           -fforce-emit-vtables -ffunction-sections
# linker flags
LDFLAGS := -lm
OPTLDFLAGS := -flto=full -fwhole-program-vtables -fvirtual-function-elimination \
              -fuse-ld=lld -Wl,--gc-sections,--icf=all -s
DEBUGFLAGS := -g3
ASMFLAGS := -S -masm=intel
DEPFLAGS = -MM -MP -MF $(DEPDIR)/$*.d

# Enables macro in the source
CFLAGS += -DVERSION=\"$(shell git describe --tags --always 2>/dev/null || echo "unknown")\"
CFLAGS += -DDATE=\"$(shell date -I)\"
CFLAGS += -DLOGLEVEL=$(LOGLEVEL)

ifdef ASAN
  CFLAGS += -fsanitize=$(ASAN)
  LDFLAGS += -fsanitize=$(ASAN)
endif

ifdef LLVM
  ASMFLAGS += -emit-llvm
endif

ifeq ($(TYPE),test)
  CFLAGS += -DTEST_MODE
else ifeq ($(TYPE),bench)
  CFLAGS += -DBENCHMARK_MODE
else ifeq ($(TYPE),asm)
  CFLAGS += $(ASMFLAGS)
endif

ifeq ($(OPTLEVEL),g)
  CFLAGS += $(DEBUGFLAGS)
  RUNNER ?= gdb
else ifneq ($(OPTLEVEL),0)
  CFLAGS += $(OPTFLAGS)
  LDFLAGS += $(OPTLDFLAGS)
endif

ifdef TEST_FILTER
  CFLAGS += -DTEST_FILTER="\"$(TEST_FILTER)\""
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

ifeq ($(MAKECMDGOALS),build)
	-include $(DEPS)
else ifeq ($(MAKECMDGOALS),run)
	-include $(DEPS)
endif

# rules
.PHONY: run analyze clean-all clean install doc test lint fmt help release log llmfile
.DEFAULT_GOAL := build

build: $(TARGET)

# link
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(EXTRALDFLAGS) $^ -o $@

# compile
$(TARGETDIR)/%.o: $(SRCDIR)/%.c $(DEPDIR)/%.d | $(TARGETDIR)/
	$(CC) $< -I$(INCDIR) $(CFLAGS) $(EXTRAFLAGS) -c -o $@

$(DEPDIR)/%.d: $(SRCDIR)/%.c | $(DEPDIR)/
	$(CC) $< -I$(INCDIR) $(CFLAGS) $(DEPFLAGS)

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
	$(MAKE) run TYPE=test

clean-all:
	rm -rf $(BUILDDIR)

# e.g.) remove test build for opt level 3
# make clean OPTLEVEL=3 TYPE=test
clean:
	rm -rf $(OUTDIR)

install: $(TARGET) | $(PREFIX)/bin/
	cp $^ $(PREFIX)/bin/

uninstall:
	rm $(PREFIX)/bin/$(PROJECT_NAME)

# generate doc
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
	$(MAKE) run TYPE=test OPTLEVEL=3
	$(MAKE) lint
	$(MAKE) OPTLEVEL=3

LLMFILE ?= llmfile.txt
LIST_FILES ?= README.md makefile include/* src/*
llmfile:
	echo $(wildcard $(LIST_FILES)) | sed 's/ /\n/g' > $(LLMFILE)
	echo >> $(LLMFILE)
	head -n 9999 $(LIST_FILES) >> $(LLMFILE)
	@echo "Generated $(LLMFILE)"
