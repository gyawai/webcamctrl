CC=gcc
CFLAGS=-g
TARGET=webcamctrl
SRCS=main.c pad.c key.c

all:	$(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -lncurses

clean:
	/usr/bin/rm *.o
