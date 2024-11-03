CC = clang -std=c23
DB = lldb
CFLAGS = -I$(INCDIR) -Wtautological-compare -Wsign-compare -Wall -Wextra -flto=full -fwhole-program-vtables -fforce-emit-vtables -fvirtual-function-elimination -ffunction-sections -fdata-sections -faddrsig -march=native -mtune=native
LDFLAGS = -lm -fuse-ld=lld
OPTFLAGS = -O3 -ffast-math -fno-finite-math-only -DNDEBUG -Wl,--gc-sections -Wl,--icf=all -s
TSTFLAGS = -DTEST_MODE -fsanitize=address -fcoverage-mapping -fprofile-instr-generate
DEBUGFLAGS = -g -fsanitize=address -D_FORTIFY_SOURCE=2
SRCDIR = src
INCDIR = include
TSTDIR = test
BUILDDIR = .build
OUTFILE = rpx
ASMFLAGS = -S -masm=intel

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

.PHONY: all default build debug test asm run analyze clean

default: build run

build:
	$(CC) $(SRCDIR)/*.c $(CFLAGS) $(OPTFLAGS) $(LDFLAGS) -o $(OUTFILE)

debug:
	clang-18 -std=c23 $(SRCDIR)/*.c $(DEBUGFLAGS) $(LDFLAGS) -o $(OUTFILE)
	$(DB) $(OUTFILE)

test:
	clang-18 -std=c23 $(SRCDIR)/*.c $(CFLAGS) $(TSTFLAGS) $(LDFLAGS) -I$(INCDIR) -o $(OUTFILE)
	./$(OUTFILE)

asm: $(SRCS)
	$(CC) -I$(INCDIR) $(ASMFLAGS) $(SRCS)

run:
	./$(OUTFILE)

analyze:
	clang-tidy $(SRCS) -- $(CFLAGS) -std=c23

clean:
	rm -rf $(BUILDDIR) $(OUTFILE)

install: build
	cp $(OUTFILE) /usr/local/bin/

doc:
	doxygen doc/Doxyfile

release:
	$(MAKE) test
	$(MAKE) analyze
	$(MAKE) build
