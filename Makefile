all:
	g++ -Isrc -Isrc/core -Isrc/scene -Isrc/entities \
		src/main.cpp src/game.cpp \
		src/core/map.cpp src/core/light.cpp src/core/props.cpp src/core/raycast.cpp \
		src/scene/scene.cpp src/scene/door.cpp src/scene/bonus.cpp \
		src/entities/player.cpp src/entities/zombie.cpp src/entities/weapon.cpp src/entities/spawn.cpp \
		src/level.cpp \
		-o main -lraylib -lX11 -lXi -lXrandr -lXinerama -lXcursor -lGL -lm
	./main
