CC=g++
CFLAGS=-g -Wall -Wextra -pedantic -std=c++23

SRC=$(wildcard src/*.cpp)
OBJ=$(SRC:%.cpp=%.o)
HDR=$(wildcard src/*.hpp)

EXE=acchording
LIBS=$(addprefix -l,fmt hpdf fontconfig)

TARGET=/usr/local

CONF=src/config.hpp
CONFDEF=src/config.def.hpp

all: $(EXE)

$(CONF):
	cp $(CONFDEF) $(CONF)

install: all
	cp $(EXE) $(TARGET)/bin

clean:
	rm $(OBJ) $(EXE) $(CONF)

$(EXE): $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

$(OBJ): $(HDR) $(CONF)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
