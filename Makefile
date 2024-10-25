CC=g++
CFLAGS=-g -Wall -Wextra -pedantic -std=c++23

SRC=$(wildcard src/*.cpp)
OBJ=$(SRC:%.cpp=%.o)
HDR=$(wildcard src/*.hpp)

EXE=acchording
LIBS=$(addprefix -l,fmt hpdf fontconfig)

TARGET=/usr/local

all: $(EXE)

install: all
	cp $(EXE) $(TARGET)/bin

clean:
	rm $(OBJ) $(EXE)

$(EXE): $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

$(OBJ): $(HDR)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
