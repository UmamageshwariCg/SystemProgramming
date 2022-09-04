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

all: Client

Client: $(OBJ)/MainClient.o $(OBJ)/Client.o 
	$(CC) $(LFLAGS) $(BIN)/Client $(OBJ)/MainClient.o $(OBJ)/Client.o

$(OBJ)/MainClient.o: $(SRC)/MainClient.cpp
	$(CC) $(CFLAGS) $(IFLAG) $(SRC)/MainClient.cpp 
	mv *.o $(OBJ)

$(OBJ)/Client.o: $(SRC)/Client.cpp
	$(CC) $(CFLAGS) $(IFLAG) $(SRC)/Client.cpp 
	mv *.o $(OBJ)


clean:
	@echo "Cleaning the Project"
	rm -f *.o a.out
	rm -f $(OBJ)/*.o
	rm -f $(BIN)/Client
	@echo "Cleaning Done!" 

clearlogs:
	rm -f $(LOG)/*.log

valgrind:
	valgrind --leak-check=full $(BIN)/Client
	mv valgrind $(BIN)
