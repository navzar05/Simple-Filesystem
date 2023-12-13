CC=g++
CFLAGS=-Wall -I./includes
LDFLAGS=-L./lib -ldisk -lfilesystem
AR=ar
ARFLAGS=rcs
SRC_DIR=src/library
LIB_DIR=lib
BIN_DIR=bin

.PHONY: all clean

all: $(BIN_DIR)/main

$(BIN_DIR)/main: $(SRC_DIR)/main.o $(LIB_DIR)/libdisk.a $(LIB_DIR)/libfilesystem.a
	$(CC) -static $(SRC_DIR)/main.o -o $(BIN_DIR)/main $(LDFLAGS)

$(SRC_DIR)/main.o: $(SRC_DIR)/main.cpp
	$(CC) $(CFLAGS) -c -o $(SRC_DIR)/main.o $(SRC_DIR)/main.cpp

$(LIB_DIR)/libdisk.a: $(SRC_DIR)/disk_driver.o
	$(AR) $(ARFLAGS) $(LIB_DIR)/libdisk.a $(SRC_DIR)/disk_driver.o

$(SRC_DIR)/disk_driver.o: $(SRC_DIR)/disk_driver.cpp
	$(CC) $(CFLAGS) -c -o $(SRC_DIR)/disk_driver.o $(SRC_DIR)/disk_driver.cpp

$(LIB_DIR)/libfilesystem.a: $(SRC_DIR)/filesystem.o $(SRC_DIR)/fileSystemAPI.o
	$(AR) $(ARFLAGS) $(LIB_DIR)/libfilesystem.a $(SRC_DIR)/filesystem.o $(SRC_DIR)/fileSystemAPI.o

$(SRC_DIR)/fileSystemAPI.o: $(SRC_DIR)/fileSystemAPI.cpp
	$(CC) $(CFLAGS) -c -o $(SRC_DIR)/fileSystemAPI.o $(SRC_DIR)/fileSystemAPI.cpp

$(SRC_DIR)/filesystem.o: $(SRC_DIR)/filesystem.cpp
	$(CC) $(CFLAGS) -c -o $(SRC_DIR)/filesystem.o $(SRC_DIR)/filesystem.cpp

clean:
	rm -f $(SRC_DIR)/*.o $(LIB_DIR)/*.a $(BIN_DIR)/main
