# TYPE = test                      Enables tests
# TYPE = bench                     Enables benchmarks
# TYPE = asm                       Generates assembly
# OPTLEVEL = [0-3g]                Sets optimization/debug
# LOGLEVEL = [1| ]                 Enables logging
# ASAN = [0|address|alignment|...] Enables specified sanitizer

.PHONY: run analyze clean-all clean install doc test
MAKEFLAGS += -j$(shell nproc)

# Alias
OPTLEVEL ?= $(OL)
LOGLEVEL ?= $(LL)
TYPE ?= $(T)

OPTLEVEL ?= g

CC := ccache clang-18
RUNNER :=

SRCDIR := src
INCDIR := include
BUILDDIR := .build

CFLAGS := -std=c23 -I$(INCDIR) -Wtautological-compare -Wsign-compare -Wall    \
          -Wextra -fforce-emit-vtables -ffunction-sections -fdata-sections    \
		  -faddrsig -march=native -mtune=native -O$(OPTLEVEL)
LDFLAGS := -lm -flto=full -fwhole-program-vtables -fvirtual-function-elimination -fuse-ld=lld
OPTFLAGS = -ffast-math -fno-finite-math-only -DNDEBUG
DEBUGFLAGS := -g3
ASMFLAGS := -S -masm=intel

# Enables macro in the source
CFLAGS += -DVERSION=\"$(shell git describe --tags --always 2>/dev/null || echo "unknown")\"
CFLAGS += -DDATE=\"$(shell date -I)\"

ifdef LOGLEVEL
  CFLAGS += -DICECREAM
endif

ifeq ($(TYPE),test)
  CFLAGS += -DTEST_MODE
  ASAN ?= address
endif

ifeq ($(TYPE),bench)
  CFLAGS += -DBENCHMARK_MODE
endif

ifeq ($(TYPE),asm)
  CFLAGS = $(ASMFLAGS)
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
  LDFLAGS += -Wl,--gc-sections -Wl,--icf=all -s
endif

# Build rules
GITBRANCH := $(shell git branch --show-current 2>/dev/null)
HASH := $(shell echo '$(TYPE)$(OPTLEVEL)$(LOGLEVEL)$(ASAN)$(GITBRANCH)' | md5sum | cut -d' ' -f1)
OUTDIR := $(BUILDDIR)/$(HASH)
TARGETDIR := $(OUTDIR)/target
DEPDIR := $(OUTDIR)/dep
TARGET := $(TARGETDIR)/$(notdir $(PWD))
.DEFAULT_GOAL := $(TARGET)

$(TARGETDIR):
	mkdir -p $@

$(DEPDIR):
	mkdir -p $@

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(TARGETDIR)/%.o,$(SRCS))
DEPS = $(patsubst $(SRCDIR)/%.c,$(DEPDIR)/%.d,$(SRCS))

-include $(DEPS)

$(TARGET): $(OBJS)
	@echo "Linking $@"
	@$(CC) $(LDFLAGS) $^ -o $@

$(TARGETDIR)/%.o: $(SRCDIR)/%.c | $(TARGETDIR) $(DEPDIR)
	@echo "Compiling $<"
	@$(CC) $< -I$(INCDIR) $(CFLAGS) $(EXTRAFLAGS) -MMD -MF $(DEPDIR)/$*.d -c -o $@

run: $(TARGET)
	$(RUNNER) $<

run-%: $(TARGET)
	$* $<

clean-all:
	rm -rf $(BUILDDIR)

clean:
	rm -rf $(OUTDIR)

install: $(TARGET)
	cp $^ /usr/local/bin/

doc: doc/Doxyfile
	doxygen $<

fmt:
	clang-format -i $(SRCS)

lint:
	clang-tidy $(SRCS) -- $(CFLAGS)

FP ?= /dev/stdout
log:
	@echo "Compiler: $(CC)" > $(FP)
	@echo "CFLAGS: $(CFLAGS)" >> $(FP)
	@echo "LDFLAGS: $(LDFLAGS)" >> $(FP)
	@echo "TARGET: $(TARGET)" >> $(FP)

release:
	$(MAKE) TYPE=test OPTLEVEL=3
	$(MAKE) lint
	$(MAKE)
