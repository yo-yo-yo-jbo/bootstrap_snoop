CC=gcc
CFLAGS=-Wall
LDGLAGS=

all: bootstrap_snoop

bootstrap_snoop: bootstrap_snoop.o
		$(CC) $(CFLAGS) bootstrap_snoop.o -o bootstrap_snoop $(LDFLAGS)

clean:
		rm bootstrap_snoop.o bootstrap_snoop
