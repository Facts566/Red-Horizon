all:
	g++ src/main.cpp -o main -lraylib -lX11 -lXi -lXrandr -lXinerama -lXcursor -lGL -lm
	./main