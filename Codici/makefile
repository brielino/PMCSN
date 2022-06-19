CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lm

OBJFILES = rngs.o rvgs.o Autofficina.o ValoriTeorici.o AutofficinaAggiornata.o Transiente.o

all: $(OBJFILES)

ValoriTeorici.o: ValoriTeorici.c
	$(CC) $^ -o $@ $(LDFLAGS)

rngs.o: rngs.c rngs.h
	$(CC) -c $<

rvgs.o: rvgs.c rvgs.h rngs.c rngs.h
	$(CC) -c $<

rvms.o: rvms.c rvms.h
	$(CC) -c $<

Autofficina.o: Autofficina.c rngs.o rvgs.o
	$(CC) $^ -o $@ $(LDFLAGS)

AutofficinaAggiornata.o: AutofficinaAggiornata.c rngs.o rvgs.o
	$(CC) $^ -o $@ $(LDFLAGS)

Transiente.o: Transiente.c rngs.o rvgs.o
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	/bin/rm -f $(OBJFILES) core*
