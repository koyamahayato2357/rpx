NPROC != nproc
MAKEFLAGS += -j$(NPROC)
MAKEFLAGS += -r -R

PHONY_TARGETS != grep -o "^[0-9a-z-]\\+:" $(MAKEFILE_LIST) | sed -e "s/://"
.PHONY: $(PHONY_TARGETS)

define ERROR_INVALID_VALUE
  $(error invalid value for $$($1): expected one of $(or $2,[yn]) but got $($1))
endef

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

OPTLEVEL ?= g ## optimization level [0-3|g] (default: g)
TYPE ?= default ## build type [test|bench|default]

CLANG21 != command -v clang-21
CLANG20 != command -v clang-20
CLANG19 != command -v clang-19

CC := $(or $(CLANG21),$(CLANG20),$(CLANG19),$(error CC not found))

DISABLE_CCACHE ?= n ## disable ccache [yn] (default: n)
ifeq ($(strip $(DISABLE_CCACHE)),n)
  CCACHE != command -v ccache
  CC := $(CCACHE) $(CC)
else ifeq ($(strip $(DISABLE_CCACHE)),y)
else
  $(call ERROR_INVALID_VALUE,DISABLE_CCACHE)
endif

PROJECT_NAME := $(notdir $(CURDIR))
CDIR := src
INCDIR := include
BUILDDIR := .build
PREFIX ?= /usr/local ## install prefix (default: /usr/local)

# compiler flags
CFLAGS := -std=c2y -I$(INCDIR) -O$(OPTLEVEL)

WARNFLAGS := tautological-compare extra all error implicit-fallthrough \
			 bitwise-instead-of-logical conversion dangling deprecated \
			 documentation microsoft switch-enum switch-default type-limits \
			 unreachable-code-aggressive sign-compare pedantic documentation-pedantic
WARNNOFLAGS := dollar-in-identifier-extension gnu
SECURITYFLAGS := PIE no-plt

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
DEPFLAGS = -MM -MP -MT $(OUTDIR)/$*.o -MF $(OUTDIR)/$*.d

# Enables macro in the source
VERSION != git describe --tags --always 2>/dev/null || echo "unknown"
DATE != date -I
CFLAGS += -DVERSION=\"$(VERSION)\"
CFLAGS += -DDATE=\"$(DATE)\"
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

ifeq ($(strip $(TYPE)),test)
  CFLAGS += -DTEST_MODE
else ifeq ($(strip $(TYPE)),bench)
  CFLAGS += -DBENCHMARK_MODE
else ifeq ($(strip $(TYPE)),default)
else
  $(call ERROR_INVALID_VALUE,TYPE,[test|bench])
  $(call ERROR_INVALID_VALUE,TYPE,[test|bench|default])
endif

ifeq ($(strip $(OPTLEVEL)),g)
  CFLAGS += $(DEBUGFLAGS)
  LDFLAGS += $(DEBUGFLAGS)
  RUNNER ?= gdb ## runner (default in debug run: gdb)
else ifneq ($(filter 1 2 3,$(OPTLEVEL)),)
  CFLAGS += $(OPTFLAGS)
  LDFLAGS += $(OPTLDFLAGS)
else ifeq ($(strip $(OPTLEVEL)),0)
else
  $(call ERROR_INVALID_VALUE,OPTLEVEL,[0-3|g])
endif

ifeq ($(MAKECMDGOALS),profile)
  CFLAGS += -pg
  LDFLAGS += -pg
  CFLAGS := $(filter-out -fomit-frame-pointer,$(CFLAGS))
  LDFLAGS := $(filter-out -s,$(LDFLAGS))
endif

ifdef TEST_FILTER
  CFLAGS += -DTEST_FILTER="\"$(TEST_FILTER)\""
endif

# generate output path
GITBRANCH != git branch --show-current 2>/dev/null
SEED = $(CC)$(EXTRAFLAGS)$(CFLAGS)$(LDFLAGS)$(GITBRANCH)
HASH != echo '$(SEED)' | md5sum | cut -d' ' -f1
OUTDIR := $(BUILDDIR)/$(HASH)

TARGET := $(OUTDIR)/$(PROJECT_NAME)

EMIT_LLVM ?= n ## use llvmIR instead of asm [yn] (default: n)
ifeq ($(strip $(EMIT_LLVM)),y)
  ASMFLAGS += -emit-llvm
  ASMEXT := ll
else ifeq ($(strip $(EMIT_LLVM)),n)
  ASMEXT := s
else
  $(call ERROR_INVALID_VALUE,EMIT_LLVM)
endif

# source files
SRCS := $(wildcard $(CDIR)/*.c)
OBJS := $(patsubst $(CDIR)/%.c,$(OUTDIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)
ASMS := $(OBJS:.o=.$(ASMEXT))

# e.g.)
# $ make asm OL=3
# $ # edit asm files...
# $ make BUILD_FROM_ASM=y OL=3
BUILD_FROM_ASM ?= n ## use asm instead of c files [yn] (default: n)
ifeq ($(strip $(BUILD_FROM_ASM)),y)
  SRCDIR := $(OUTDIR)
  SRCEXT := $(ASMEXT)
  CFLAGS =
else ifeq ($(strip $(BUILD_FROM_ASM)),n)
  SRCDIR := $(CDIR)
  SRCEXT := c
else
  $(call ERROR_INVALID_VALUE,BUILD_FROM_ASM)
endif

ifneq ($(filter $(TARGET) run, $(MAKECMDGOALS)),)
  include $(wildcard $(DEPS))
endif

### build rules
.DEFAULT_GOAL := $(TARGET)

# link
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(EXTRALDFLAGS) $^ -o $@

# compile
$(OBJS): $(OUTDIR)/%.o: $(SRCDIR)/%.$(SRCEXT) $(OUTDIR)/%.d | $(OUTDIR)/
	$(CC) $< $(CFLAGS) $(EXTRAFLAGS) -c -o $@

$(DEPS): $(OUTDIR)/%.d: $(SRCDIR)/%.c | $(OUTDIR)/
	$(CC) $< $(CFLAGS) $(EXTRAFLAGS) $(DEPFLAGS) -o $@

# e.g.) run with valgrind
# make run RUNNER=valgrind
# e.g.) don't use gdb (default debug RUNNER) in debug run
# make run RUNNER=
run: $(TARGET) ## run target
	$(RUNNER) $<

# `make run-foo` is same as `make run RUNNER=foo`
run-%: $(TARGET)
	$* $<

test: ; $(MAKE) run TYPE=test RUNNER= ## run test

asm: $(ASMS) ## generate asm files

$(ASMS): $(OUTDIR)/%.$(ASMEXT): $(SRCDIR)/%.c | $(OUTDIR)/
	$(CC) $< $(ASMFLAGS) $(CFLAGS) $(EXTRAFLAGS) -o $@

clean-all: ; rm -rf $(BUILDDIR)

# e.g.) remove test build for opt level 3
# make clean OPTLEVEL=3 TYPE=test
clean:
ifneq ($(realpath $(OUTDIR)),)
	rm -rf $(OUTDIR)
endif

install-bin: $(TARGET) | $(PREFIX)/bin/
	cp $^ $|

install-example: | $(HOME)/.config/$(PROJECT_NAME)/
	cp example/* $|

install: install-bin install-example

uninstall:
	rm $(PREFIX)/bin/$(PROJECT_NAME)

doc: doc/Doxyfile ## generate doc
	doxygen $<

doc/Doxyfile: ; doxygen -g $@

fmt: ; clang-format -Werror --dry-run $(SRCS) $(INCDIR)/*.h

lint:
	clang-tidy $(SRCS) -- $(CFLAGS)
	cppcheck $(SRCS) --enable=all --suppress=missingIncludeSystem -I$(INCDIR)
	scan-build $(MAKE)

pre-commit: fmt test ## .git/hooks/pre-commit

FP ?= /dev/stdout
log: ## show build flags
	@echo "Compiler: $(CC)" > $(FP)
	@echo "CFLAGS: $(CFLAGS)" >> $(FP)
	@echo "LDFLAGS: $(LDFLAGS)" >> $(FP)
	@echo "TARGET: $(TARGET)" >> $(FP)
	@echo "SRCS: $(SRCS)" >> $(FP)
	@echo "OBJS: $(OBJS)" >> $(FP)
	@echo "DEPS: $(DEPS)" >> $(FP)

info: $(TARGET) ## show target info
	@echo "target file size:"
	@size $(TARGET)

help: ## show help
	@echo "$ make          # debug build"
	@echo "$ make test     # run test"
	@echo "$ make run OL=3 # run release build"
	@echo
	@echo "build files: .build/HASH/*"
	@echo
	@echo "Variables:"
	@grep "^[^\t].* ## " $(MAKEFILE_LIST) \
	| sed -En "s/^ *([0-9A-Z_]+) .?= .*## (.*)$$/\\1 = \\2/p"
	@echo
	@echo "Targets:"
	@grep "^[^\t].* ## " $(MAKEFILE_LIST) \
	| sed -En "s/^([0-9a-z-]+): .*## (.*)$$/\\1: \\2/p" \

### llmfile

LLMFILE ?= llmfile.txt
FILES ?= README.md makefile build.zig idea.txt
DIRS ?= include src
FILES_IN_DIRS := $(wildcard $(addsuffix /*, $(DIRS)))
SORTED_FILES_IN_DIRS := $(sort $(notdir $(basename $(FILES_IN_DIRS))))
REAL_PATH_FILES_IN_DIRS := $(foreach f,$(SORTED_FILES_IN_DIRS),$(shell find $(DIRS) -name $f.?))
LIST_FILES ?= $(FILES) $(REAL_PATH_FILES_IN_DIRS)
$(LLMFILE): $(LIST_FILES)
	echo $^ | sed 's/ /\n/g' > $@
	echo >> $@ # newline
	# `head` automatically inserts the file name at the start of the file
	head -n 9999 $^ >> $@

llmfile: $(LLMFILE) ## for the llm to read

### compiledb

compile_commands.json: $(SRCS)
	$(MAKE) clean
	bear -- $(MAKE)

compiledb: compile_commands.json ## for lsp

### coverage

GCOV_TOOL ?= $(CURDIR)/tool/llvm-cov.sh
COVDIR ?= coverage-report

$(GCOV_TOOL): | $(dir $(GCOV_TOOL))
	echo -e '#!/bin/sh\nexec llvm-cov gcov "$$@"' > $@
	chmod +x $@

GCDA_FILES := $(OBJS:.o=.gcda)
$(GCDA_FILES): run

%/$(COVDIR).info: $(GCOV_TOOL) $(GCDA_FILES)
	lcov -d $* -c -o $@ --gcov-tool $<

%/$(COVDIR): %/$(COVDIR).info
	genhtml $< -o $@

BROWSER ?= w3m # w3m is sufficient for viewing
coverage: $(OUTDIR)/$(COVDIR) ## report test coverage
	$(BROWSER) $</index.html

### profile

$(OUTDIR)/profile.txt: run
	gprof $(TARGET)

profile: $(OUTDIR)/profile.txt
	less $<

%/: ; mkdir -p $@
