.PHONY: all clean

SRCDIR := src
INCDIR := include
BUILDDIR := build
TARGET := main
CC := gcc
CFLAGS := -I$(INCDIR)
LDFLAGS := -lm

SRC := $(wildcard $(SRCDIR)/*.c)
OBJ := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRC))

all: $(TARGET) diff

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILDDIR):
	mkdir -p $@

diff.o: tools/diff.c
	$(CC) -c tools/diff.c -o diff.o $(CFLAGS)

diff: diff.o
	$(CC) -o diff diff.o $(LDFLAGS)

clean:
	rm -rf $(TARGET) $(BUILDDIR) diff diff.o

