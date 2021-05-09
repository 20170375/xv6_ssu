CC = gcc
RM = rm -f
CFLAGS = -o

TARGET = fork exec cat pipe 

.PHONY: all clean

all: $(TARGET)

clean:
	$(RM) $(TARGET)

$(TARGET):
	$(CC) fork.c $(CFLAGS) fork
	$(CC) exec.c $(CFLAGS) exec
	$(CC) cat.c $(CFLAGS) cat
	$(CC) pipe.c $(CFLAGS) pipe