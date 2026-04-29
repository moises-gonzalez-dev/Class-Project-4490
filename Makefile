all: project

project: project.cpp
	g++ project.cpp math_utils.cpp geometry.cpp -Wall -lX11 -lGL -lGLU -lm ./libggfonts.a -lopenal -lalut -o project

clean:
	rm -f project
