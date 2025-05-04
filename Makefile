# set compile flags
FLAGS = -O3 -DNDEBUG -g -Wall -march=native -std=c++20

.PHONY: clean all

all: Calc_Energy

Calc_Energy: Calc_Energy.cpp
	mpic++ $(FLAGS) -o Calc_Energy Calc_Energy.cpp

clean:
	rm -f Calc_Energy.o
