CC = g++
LFLAGS = -o
//CFLAGS = -c -g -Wall -Werror
INC = ./inc
IFLAG = -I$(INC)
BIN = ./bin
OBJ = ./obj
SRC = ./src
LOG = ./log

CVFLAGS = -v --tool=memcheck --leak-check=full --show-reachable=yes --log-file=valclient

all: Server

Server: $(OBJ)/MainServer.o $(OBJ)/Server.o 
	$(CC) $(LFLAGS) $(BIN)/Server $(OBJ)/MainServer.o $(OBJ)/Server.o

$(OBJ)/MainServer.o: $(SRC)/MainServer.cpp
	$(CC) $(CFLAGS) $(IFLAG) $(SRC)/MainServer.cpp 
	mv *.o $(OBJ)

$(OBJ)/Server.o: $(SRC)/Server.cpp
	$(CC) $(CFLAGS) $(IFLAG) $(SRC)/Server.cpp 
	mv *.o $(OBJ)


clean:
	@echo "Cleaning the Project"
	rm -f *.o a.out
	rm -f $(OBJ)/*.o
	rm -f $(BIN)/Server
	@echo "Cleaning Done!" 

clearlogs:
	rm -f $(LOG)/*.log

valgrind:
	valgrind --leak-check=full $(BIN)/s
	mv valgrind $(BIN)
