# TYPE = test                      Enables tests
# TYPE = bench                     Enables benchmarks
# TYPE = asm                       Generates assembly
# OPTLEVEL = [0-3|g]               Sets optimization/debug
# LOGLEVEL = [1| ]                 Enables logging
# ASAN = [0|address|alignment|...] Enables specified sanitizer

.PHONY: run analyze clean-all clean install doc test lint fmt help release log
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

CCACHE := $(shell which ccache 2>/dev/null)
CC := $(CCACHE) clang
RUNNER :=

SRCDIR := src/
INCDIR := include/
BUILDDIR := .build/
INSTALLDIR ?= /usr/local/

CFLAGS := -std=c23 -I$(INCDIR) -Wtautological-compare -Wsign-compare -Wall    \
          -Wextra -fforce-emit-vtables -ffunction-sections -fdata-sections    \
		  -faddrsig -march=native -mtune=native -funroll-loops -fomit-frame-pointer -O$(OPTLEVEL)
LDFLAGS := -lm
OPTFLAGS = -ffast-math -fno-finite-math-only -DNDEBUG
OPTLDFLAGS := -flto=full -fwhole-program-vtables -fvirtual-function-elimination -fuse-ld=lld -Wl,--gc-sections -Wl,--icf=all -s
DEBUGFLAGS := -g3
ASMFLAGS := -S -masm=intel

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
  RUNNER := lldb
else ifneq ($(OPTLEVEL),0)
  CFLAGS += $(OPTFLAGS)
  LDFLAGS += $(OPTLDFLAGS)
endif

# Build rules
GITBRANCH := $(shell git branch --show-current 2>/dev/null)
HASH := $(shell echo '$(TYPE)$(OPTLEVEL)$(LOGLEVEL)$(ASAN)$(GITBRANCH)' | md5sum | cut -d' ' -f1)
OUTDIR := $(BUILDDIR)/$(HASH)/
TARGETDIR := $(OUTDIR)/target/
DEPDIR := $(OUTDIR)/dep/
TARGET := $(TARGETDIR)/$(notdir $(shell pwd))
DEPFLAGS := -MMD -MF $(DEPDIR)/$*.d
.DEFAULT_GOAL := $(TARGET)

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(TARGETDIR)/%.o,$(SRCS))
DEPS = $(patsubst $(SRCDIR)/%.c,$(DEPDIR)/%.d,$(SRCS))

-include $(DEPS)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

$(TARGETDIR)/%.o: $(SRCDIR)/%.c | $(TARGETDIR) $(DEPDIR)
	$(CC) $< -I$(INCDIR) $(CFLAGS) $(EXTRAFLAGS) $(DEPFLAGS) -c -o $@

%/:
	mkdir -p $@

run: $(TARGET)
	$(RUNNER) $<

run-%: $(TARGET)
	$* $<

clean-all:
	rm -rf $(BUILDDIR)

clean:
	rm -rf $(OUTDIR)

install: $(TARGET) | $(INSTALLDIR)
	cp $^ $(INSTALLDIR)bin/

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
	$(MAKE)
