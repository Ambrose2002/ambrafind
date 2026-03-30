IDIR = ../include
LIBS = ../lib
CC = gcc
CFLAGS = -I$(IDIR) -I$(LIBS)

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS += -g -DDEBUG
else
    CFLAGS += -O2
endif

ODIR = obj
TARGET = ../ambrafind.out

_DEPS = index.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = index.o main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

.PHONY: all clean

all: $(TARGET)

$(ODIR):
	mkdir -p $(ODIR)

$(ODIR)/%.o: %.c $(DEPS) | $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(ODIR)/*.o $(TARGET) *~ core $(IDIR)/*~
