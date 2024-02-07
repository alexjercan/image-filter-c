.PHONY: all clean

SRCDIR := src
INCDIR := include
BUILDDIR := build
TARGET := main
CC := gcc
CFLAGS := -I$(INCDIR)
LDFLAGS := -lm -L/opt/cuda/lib64/ -lcudart

SRC := $(wildcard $(SRCDIR)/*.c)
OBJ := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRC))

SRC_CU := $(wildcard $(SRCDIR)/*.cu)
OBJ_CU := $(patsubst $(SRCDIR)/%.cu,$(BUILDDIR)/%_cu.o,$(SRC_CU))

all: $(TARGET) diff

$(TARGET): $(OBJ) $(OBJ_CU)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILDDIR)/%_cu.o: $(SRCDIR)/%.cu | $(BUILDDIR)
	nvcc -c $< -o $@ $(CFLAGS)

$(BUILDDIR):
	mkdir -p $@

diff.o: tools/diff.c
	$(CC) -c tools/diff.c -o diff.o $(CFLAGS)

diff: diff.o
	$(CC) -o diff diff.o $(LDFLAGS)

clean:
	rm -rf $(TARGET) $(BUILDDIR) diff diff.o

