CC = gcc
CFLAGS = -g -c
LDLAGS = -g
TARGET = webcamctrl
SRCS = main.c pad.c key.c server.c client.c utils.c
OBJS = $(SRCS:%.c=%.o)

all:	$(TARGET)

$(TARGET):	$(OBJS)
	$(CC) $(LDLAGS) -o $(TARGET) $(OBJS) -lncurses

.c.o:
	$(CC) $(CFLAGS) -o $@ $<

clean:
	-/usr/bin/rm *.o

allclean:
	make clean
	-/usr/bin/rm $(TARGET)
