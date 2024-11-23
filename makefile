# TYPE = test                      Enables tests
# TYPE = bench                     Enables benchmarks
# TYPE = asm                       Generates assembly
# OPTLEVEL = [0-3g]                Sets optimization/debug
# LOGLEVEL = [1| ]                 Enables logging
# ASAN = [0|address|alignment|...] Enables specified sanitizer

CC := clang-18
RUNNER :=

SRCDIR := src
INCDIR := include
BUILDDIR = build

CFLAGS := -std=c23 -I$(INCDIR) -Wtautological-compare -Wsign-compare -Wall    \
          -Wextra -fforce-emit-vtables -ffunction-sections -fdata-sections    \
		  -faddrsig -march=native -mtune=native
LDFLAGS := -lm -flto=full -fwhole-program-vtables -fvirtual-function-elimination -fuse-ld=lld
OPTFLAGS = -ffast-math -fno-finite-math-only -DNDEBUG
DEBUGFLAGS := -g3
ASMFLAGS := -S -masm=intel

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

all: run

ifdef LOGLEVEL
  CFLAGS += -DICECREAM
endif

ifeq ($(TYPE),test)
  OPTLEVEL ?= g
  CFLAGS += -DTEST_MODE
  RUNNER =
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

OPTLEVEL ?= 3

ifeq ($(OPTLEVEL),g)
  CFLAGS += $(DEBUGFLAGS)
  RUNNER := lldb
else
  CFLAGS += -O$(OPTLEVEL)
  ifneq ($(OPTLEVEL),0)
    CFLAGS += $(OPTFLAGS)
    LDFLAGS += -Wl,--gc-sections -Wl,--icf=all -s
  endif
endif

# Build rules
TARGETDIR := $(BUILDDIR)/T$(TYPE)-O$(OPTLEVEL)-L$(LOGLEVEL)-SAN$(ASAN)
OUTFILE := $(TARGETDIR)/rpx

$(TARGETDIR):
	mkdir -p $@

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(TARGETDIR)/%.o,$(SRCS))

$(OUTFILE): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

$(TARGETDIR)/%.o: $(SRCDIR)/%.c | $(TARGETDIR)
	$(CC) $< -I$(INCDIR) $(CFLAGS) $(EXTRAFLAGS) -c -o $@

run: $(OUTFILE)
	$(RUNNER) $<

analyze:
	clang-tidy $(SRCS) -- $(CFLAGS)

clean:
	rm -rf $(BUILDDIR) $(OUTFILE)

install: build
	cp $(OUTFILE) /usr/local/bin/

doc:
	doxygen doc/Doxyfile

release:
	$(MAKE) TYPE=test
	$(MAKE) analyze
	$(MAKE)
