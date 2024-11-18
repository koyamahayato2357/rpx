CC = clang-18 -std=c23
DB = lldb
CFLAGS = -I$(INCDIR) -Wtautological-compare -Wsign-compare -Wall -Wextra -flto=full -fwhole-program-vtables -fforce-emit-vtables -fvirtual-function-elimination -ffunction-sections -fdata-sections -faddrsig -march=native -mtune=native
LDFLAGS = -lm -fuse-ld=lld
OPTFLAGS = -O3 -ffast-math -fno-finite-math-only -DNDEBUG -Wl,--gc-sections -Wl,--icf=all -s
TSTFLAGS = -DTEST_MODE -fsanitize=address -g
BNCHFLAGS = -DBENCHMARK_MODE
DEBUGFLAGS = -g3
ICFLAGS = -DICECREAM
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
	$(CC) $(SRCDIR)/*.c -I$(INCDIR) $(DEBUGFLAGS) $(LDFLAGS) -o $(OUTFILE)
	$(DB) $(OUTFILE)

test:
	$(CC) $(SRCDIR)/*.c $(CFLAGS) $(TSTFLAGS) $(LDFLAGS) -I$(INCDIR) -o $(OUTFILE)
	./$(OUTFILE)

all-test:
	$(CC) $(SRCDIR)/*.c $(CFLAGS) $(TSTFLAGS) -DTEST_MODE_ALL $(ICFLAGS) $(LDFLAGS) -I$(INCDIR) -o $(OUTFILE)
	./$(OUTFILE)

bench:
	$(CC) $(SRCDIR)/*.c $(CFLAGS) $(BNCHFLAGS) $(OPTFLAGS) $(LDFLAGS) -o $(OUTFILE)
	./$(OUTFILE)

ice:
	$(CC) $(SRCDIR)/*.c $(CFLAGS) $(OPTFLAGS) $(ICFLAGS) $(LDFLAGS) -o $(OUTFILE)
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
