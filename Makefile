CFLAGS=-Wall -std=c++14

ifdef DEBUG
	CFLAGS+=-g3
endif

all:
	g++ $(CFLAGS) -o exec main.cpp Converter.cpp `sdl-config --cflags --libs`


