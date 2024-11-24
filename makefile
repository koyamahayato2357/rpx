# TYPE = test                      Enables tests
# TYPE = bench                     Enables benchmarks
# TYPE = asm                       Generates assembly
# OPTLEVEL = [0-3g]                Sets optimization/debug
# LOGLEVEL = [1| ]                 Enables logging
# ASAN = [0|address|alignment|...] Enables specified sanitizer

.PHONY: run analyze clean-all clean install doc test
.DEFAULT_GOAL := run
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

CC := ccache clang-18
RUNNER :=

SRCDIR := src
INCDIR := include
BUILDDIR := build

CFLAGS := -std=c23 -I$(INCDIR) -Wtautological-compare -Wsign-compare -Wall    \
          -Wextra -fforce-emit-vtables -ffunction-sections -fdata-sections    \
		  -faddrsig -march=native -mtune=native -O$(OPTLEVEL)
LDFLAGS := -lm -flto=full -fwhole-program-vtables -fvirtual-function-elimination -fuse-ld=lld
OPTFLAGS = -ffast-math -fno-finite-math-only -DNDEBUG
DEBUGFLAGS := -g3
ASMFLAGS := -S -masm=intel

# Enables macro in the source
CFLAGS += -DVERSION=\"$(shell git describe --tags --always)\"
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
OUTDIR := $(BUILDDIR)/T$(TYPE)-O$(OPTLEVEL)-L$(LOGLEVEL)-SAN$(ASAN)-B$(shell git branch --show-current)
TARGETDIR := $(OUTDIR)/target
DEPDIR := $(OUTDIR)/dep
OUTFILE := $(TARGETDIR)/rpx

$(TARGETDIR):
	mkdir -p $@

$(DEPDIR):
	mkdir -p $@

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(TARGETDIR)/%.o,$(SRCS))
DEPS = $(patsubst $(SRCDIR)/%.c,$(DEPDIR)/%.d,$(SRCS))

-include $(DEPS)

$(OUTFILE): $(OBJS)
	@echo "Linking $@"
	@$(CC) $(LDFLAGS) $^ -o $@

$(TARGETDIR)/%.o: $(SRCDIR)/%.c
	@echo "Compiling $<"
	@$(CC) $< -I$(INCDIR) $(CFLAGS) $(EXTRAFLAGS) -MMD -MF $(DEPDIR)/$*.d -c -o $@

run: $(TARGETDIR) $(DEPDIR) $(OUTFILE)
	$(RUNNER) $(OUTFILE)

clean-all:
	rm -rf $(BUILDDIR)

clean:
	rm -rf $(OUTDIR)

install: $(OUTFILE)
	cp $^ /usr/local/bin/

doc:
	doxygen doc/Doxyfile

test:
	$(MAKE) TYPE=test OPTLEVEL=g

fmt:
	clang-format -i $(SRCS)

lint:
	clang-tidy $(SRCS) -- $(CFLAGS)

FP ?= /dev/stdout
log:
	@echo "Compiler: $(CC)" > $(FP)
	@echo "CFLAGS: $(CFLAGS)" >> $(FP)
	@echo "LDFLAGS: $(LDFLAGS)" >> $(FP)

release:
	$(MAKE) TYPE=test OPTLEVEL=3
	$(MAKE) lint
	$(MAKE)
