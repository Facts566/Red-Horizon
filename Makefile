all:
	g++ src/main.cpp src/player.cpp src/map.cpp src/light.cpp src/level.cpp src/door.cpp src/raycast.cpp src/props.cpp src/weapon.cpp src/scene.cpp -o main -lraylib -lX11 -lXi -lXrandr -lXinerama -lXcursor -lGL -lm
	./main