all: project

project: project.cpp
	g++ project.cpp -Wall -lX11 -lGL -lGLU -lm ./libggfonts.a -o project

clean:
	rm -f project
	rm -f *.o
