
CC = clang
DEBUG_CFLAGS = -Wall -Wextra -Wstrict-prototypes -Werror -pedantic -g
RELEASE_CFLAGS = -Wall -Wextra -Wstrict-prototypes -Werror -pedantic
LDFLAGS = -lm

EXEC1 = huff
OBJS1 = huff.o bitwriter.o node.o pq.o
LIBS1 = io-$(shell uname -m).a

.PHONY: all debug release huff pqtest nodetest bwtest format clean

all: release

debug: CFLAGS = $(DEBUG_CFLAGS)
debug: release

release: CFLAGS = $(RELEASE_CFLAGS)
release: huff pqtest nodetest bwtest

huff: $(OBJS1) $(LIBS1)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

pqtest: pqtest.o pq.o node.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

nodetest: nodetest.o node.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

bwtest: bwtest.o bitwriter.o $(LIBS1)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^


clean:
	rm -f $(EXEC1) $(OBJS) pqtest bitwriter.o node.o pq.o huff.o pqtest.o nodetest nodetest.o bwtest bwtest.o io.o

format:
	clang-format -i --style=file *.[ch]
