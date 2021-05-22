CC = gcc
CFLAGS = -Wall -pedantic 
LDLIBS = -lpthread
OBJ = Client Server

all : $(OBJ)
	@ echo "~FINE COMPILAZIONE~"
.PHONY: all clean
clean : 
	@ echo "~RIMOZIONE FILE CREATI~"
	-rm $(OBJ)
