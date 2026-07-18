all:
	g++ src/main.cpp src/player.cpp src/map.cpp src/light.cpp src/level.cpp -o main -lraylib -lX11 -lXi -lXrandr -lXinerama -lXcursor -lGL -lm
	./main