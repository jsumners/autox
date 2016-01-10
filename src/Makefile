
C := clang
SRC := autox.c
PROGRAM := autox

all: autox.c
	$(C) -o $(PROGRAM) -lpam -lpam_misc $(SRC)
debug: autox.c
	$(C) -o $(PROGRAM) -lpam -lpam_misc -g $(SRC)
