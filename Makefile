ifeq ($(shell uname), Darwin)
	PLATFORM="MacOS"
else
	PLATFORM="Linux"
endif

all:
ifeq ($(PLATFORM), "Linux")
	g++ main.cpp --std=c++11 -O3 -pthread -lglut -lGL -lGLU -o main
else
	g++ main.cpp --std=c++11 -O3 -pthread -framework OpenGL -framework GLUT -o main
endif
debug:
	g++ main.cpp --std=c++11 -O3 -g -pthread -lglut -lGL -lGLU -o main
run:
	./main
