CC = gcc
CFLAGS = -g -Wno-unused-function
LDFLAGS = 
LIBS = -lm

DESTDIR = ./
TARGET = main

.PHONY: all $(DESTDIR)$(TARGET)

all: $(DESTDIR)$(TARGET)

$(DESTDIR)$(TARGET):
	$(CC) $(CFLAGS) -Wall $(LDFLAGS) -o $(DESTDIR)$(TARGET) $(TARGET).c $(LIBS)

clean:
	-rm -f $(TARGET).o
	-rm -f $(TARGET)
	-rm -f *.tga
