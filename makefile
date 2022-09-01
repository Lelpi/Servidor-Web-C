OBJ = ./obj
SRC = ./src
INCLUDES = ./includes
LIB = ./lib
LIBRARY = library
SRCLIB = ./srclib
EXE = .
CFLAGS = 
CC = gcc


todo: dir lib server clean

configparser.o: $(SRCLIB)/configparser.c $(INCLUDES)/configparser.h
	$(CC) $(CFLAGS) -c $(SRCLIB)/configparser.c -o $(OBJ)/configparser.o

connections.o: $(SRCLIB)/connections.c $(INCLUDES)/connections.h
	$(CC) $(CFLAGS) -c $(SRCLIB)/connections.c -o $(OBJ)/connections.o

processhttp.o: $(SRCLIB)/processhttp.c $(INCLUDES)/processhttp.h $(INCLUDES)/picohttpparser.h
	$(CC) $(CFLAGS) -c $(SRCLIB)/processhttp.c -o $(OBJ)/processhttp.o

picohttpparser.o: $(SRCLIB)/picohttpparser.c $(INCLUDES)/picohttpparser.h
	$(CC) $(CFLAGS) -c $(SRCLIB)/picohttpparser.c -o $(OBJ)/picohttpparser.o

server.o: $(SRC)/server.c $(INCLUDES)/server.h $(INCLUDES)/connections.h $(INCLUDES)/processhttp.h
	$(CC) $(CFLAGS) -c $(SRC)/server.c -o $(OBJ)/server.o

lib: connections.o configparser.o processhttp.o picohttpparser.o
	ar rcs $(LIB)/lib$(LIBRARY).a $(OBJ)/connections.o $(OBJ)/configparser.o $(OBJ)/processhttp.o $(OBJ)/picohttpparser.o
	ranlib $(LIB)/lib$(LIBRARY).a

server: server.o 
	$(CC) $(OBJ)/server.o -L$(LIB) -l$(LIBRARY) -lpthread -o $(EXE)/server.out


cleanall: clean
	rm -f $(EXE)/*.out

clean:
	rm -f $(LIB)/*.a
	rm -f $(OBJ)/*.o

dir:
	rm -r -f $(LIB)
	rm -r -f $(OBJ)
	mkdir $(OBJ)
	mkdir $(LIB)
