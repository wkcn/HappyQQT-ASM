all:
	g++ main.cpp --std=c++11 -O3 -pthread -lglut -lGL -lGLU -o main
debug:
	g++ main.cpp --std=c++11 -O3 -g -pthread -lglut -lGL -lGLU -o main
run:
	./main
