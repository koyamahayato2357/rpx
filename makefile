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

CLANG21 := $(shell command -v clang-21)
CLANG20 := $(shell command -v clang-20)
CLANG19 := $(shell command -v clang-19)

# if CC is not defined
ifeq ($(origin $(CC)),undefined)
  CC := $(or $(CLANG21),$(CLANG20),$(CLANG19),$(CC))
endif
# e.g.) disable ccache
# $ make CCACHE=
CCACHE ?= $(shell command -v ccache)
ifneq ($(CCACHE),) # if CCACHE is enabled
  CC := $(CCACHE) $(CC)
endif

PROJECT_NAME := $(notdir $(CURDIR))
SRCDIR := src
INCDIR := include
BUILDDIR := .build
PREFIX ?= /usr/local

# compiler flags
CFLAGS := -std=c2y -I$(INCDIR) -O$(OPTLEVEL)

WARNFLAGS := tautological-compare extra all error implicit-fallthrough \
			 bitwise-instead-of-logical conversion dangling deprecated \
			 documentation microsoft switch-enum switch-default type-limits \
			 unreachable-code-aggressive sign-compare pedantic documentation-pedantic
WARNNOFLAGS = dollar-in-identifier-extension gnu
SECURITYFLAGS = PIE no-plt

CFLAGS += $(addprefix -W, $(WARNFLAGS)) \
		  $(addprefix -Wno-, $(WARNNOFLAGS)) \
		  $(addprefix -f, $(SECURITYFLAGS))

OPTFLAGS := -ffast-math -fno-finite-math-only -DNDEBUG -faddrsig -march=native \
           -mtune=native -funroll-loops -fomit-frame-pointer -fdata-sections   \
           -fforce-emit-vtables -ffunction-sections

# linker flags
LDFLAGS := -lm -Wl,-z,noexecstack,-z,relro,-z,now -pie
OPTLDFLAGS := -flto=full -fwhole-program-vtables -fvirtual-function-elimination \
              -fuse-ld=lld -Wl,--gc-sections,--icf=all -s
DEBUGFLAGS := -gfull -fstandalone-debug -ftrivial-auto-var-init=pattern -fstack-protector-all
ASMFLAGS := -S -masm=intel
DEPFLAGS = -MMD -MP -MT $(TARGETDIR)/$*.o -MF $(DEPDIR)/$*.d

# Enables macro in the source
CFLAGS += -DVERSION=\"$(shell git describe --tags --always 2>/dev/null || echo "unknown")\"
CFLAGS += -DDATE=\"$(shell date -I)\"
CFLAGS += -DLOGLEVEL=$(LOGLEVEL)

ifdef ASAN
  CFLAGS += -fsanitize=$(ASAN)
  LDFLAGS += -fsanitize=$(ASAN)
endif

ifeq ($(MAKECMDGOALS),coverage)
  CFLAGS += -fprofile-arcs -ftest-coverage
  LDFLAGS += --coverage
  TYPE := test
  OPTLEVEL := 0
endif

ifeq ($(TYPE),test)
  CFLAGS += -DTEST_MODE
else ifeq ($(TYPE),bench)
  CFLAGS += -DBENCHMARK_MODE
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
ASMDIR := $(OUTDIR)/asm

TARGET := $(TARGETDIR)/$(PROJECT_NAME)

# source files
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(TARGETDIR)/%.o,$(SRCS))
DEPS = $(patsubst $(SRCDIR)/%.c,$(DEPDIR)/%.d,$(SRCS))
ASMS = $(patsubst $(SRCDIR)/%.c,$(ASMDIR)/%.$(ASMEXT),$(SRCS))

ifdef LLVM
  ASMFLAGS += -emit-llvm
  ASMEXT := ll
else
  ASMEXT := s
endif

# e.g.)
# $ make asm OL=3
# $ # edit asm files...
# $ make BUILD_FROM_ASM=1 OL=3
ifdef BUILD_FROM_ASM
  SRCPAT = $(ASMDIR)/%.$(ASMEXT)
  CFLAGS =
else
  SRCPAT = $(SRCDIR)/%.c
endif

ifneq ($(filter $(TARGET) run, $(MAKECMDGOALS)),)
  include $(wildcard $(DEPS))
endif

# rules
.PHONY: run asm clean-all clean install doc test lint fmt help log llmfile compiledb coverage
.DEFAULT_GOAL := $(TARGET)

# link
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(EXTRALDFLAGS) $^ -o $@

# compile
$(TARGETDIR)/%.o: $(SRCPAT) | $(TARGETDIR)/ $(DEPDIR)/
	$(CC) $< $(CFLAGS) $(EXTRAFLAGS) $(DEPFLAGS) -c -o $@

$(DEPS):

%/: ; mkdir -p $@

# e.g.) run with valgrind
# make run RUNNER=valgrind
# e.g.) don't use gdb (default debug RUNNER) in debug run
# make run RUNNER=
run: $(TARGET)
	$(RUNNER) $<

# `make run-foo` is same as `make run RUNNER=foo`
run-%: $(TARGET)
	$* $<

test: ; $(MAKE) run TYPE=test

asm: $(ASMS)

$(ASMDIR)/%.$(ASMEXT): $(SRCDIR)/%.c | $(ASMDIR)/
	$(CC) $< $(ASMFLAGS) $(CFLAGS) $(EXTRAFLAGS) -o $@

clean-all: ; rm -rf $(BUILDDIR)

# e.g.) remove test build for opt level 3
# make clean OPTLEVEL=3 TYPE=test
clean:
ifneq ($(OUTDIR),)
	rm -rf $(OUTDIR)
endif

install: $(TARGET) | $(PREFIX)/bin/ ~/.config/$(PROJECT_NAME)/
	cp $^ $(PREFIX)/bin/
	cp example/* ~/.config/$(PROJECT_NAME)/

uninstall:
	rm $(PREFIX)/bin/$(PROJECT_NAME)

# generate doc
doc: doc/Doxyfile
	doxygen $<

doc/Doxyfile:
	doxygen -g $@

fmt: ; clang-format -i $(SRCS) $(INCDIR)/*.h

lint:
	clang-tidy $(SRCS) -- $(CFLAGS)
	cppcheck $(SRCS) --enable=all --suppress=missingIncludeSystem -I$(INCDIR)
	scan-build $(MAKE)

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
	@echo "TYPE=[test|bench]"
	@echo "ASAN=[address|alignment|...]"
	@echo "OPTLEVEL=[0-3|g] (default: g)"
	@echo
	@echo "$ make          # debug build"
	@echo "$ make test     # run test"
	@echo "$ make run OL=3 # run release build"
	@echo
	@echo "build files: .build/HASH/{target,dep,asm}/*"

LLMFILE ?= llmfile.txt
FILES ?= README.md makefile build.zig idea.txt
DIRS ?= include src
FILES_IN_DIRS := $(wildcard $(addsuffix /*, $(DIRS)))
SORTED_FILES_IN_DIRS := $(sort $(notdir $(basename $(FILES_IN_DIRS))))
REAL_PATH_FILES_IN_DIRS := $(foreach f,$(SORTED_FILES_IN_DIRS),$(shell find $(DIRS) -name $f.?))
LIST_FILES ?= $(FILES) $(REAL_PATH_FILES_IN_DIRS)
$(LLMFILE): $(LIST_FILES) # for the LLM to read
	echo $^ | sed 's/ /\n/g' > $@
	echo >> $@ # newline
	# `head` automatically inserts the file name at the start of the file
	head -n 9999 $^ >> $@

llmfile: $(LLMFILE)

compile_commands.json: $(SRCS)
	$(MAKE) clean
	bear -- $(MAKE)

compiledb: compile_commands.json

GCOV_TOOL ?= $(CURDIR)/tool/llvm-cov.sh
COVDIR ?= coverage-report

$(GCOV_TOOL): | $(dir $@)
	echo -e '#!/bin/sh\nexec llvm-cov gcov "$$@"' > $@
	chmod +x $@

%/$(COVDIR).info: $(GCOV_TOOL) run
	lcov -d $* -c -o $@ --gcov-tool $<

%/$(COVDIR): %/$(COVDIR).info
	genhtml $< -o $@

BROWSER ?= w3m
coverage: $(TARGETDIR)/$(COVDIR)
	$(BROWSER) $</index.html
