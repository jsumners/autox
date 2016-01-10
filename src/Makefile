
SRC := autox.c
PROGRAM := autox

all: autox.c
	$(CC) $(CFLAGS) -o $(PROGRAM) -lpam -lpam_misc $(LDFLAGS) $(SRC)
debug: autox.c
	$(CC) $(CFLAGS) -o $(PROGRAM) -lpam -lpam_misc -g $(LDFLAGS) $(SRC)
