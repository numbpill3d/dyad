# native ghost / simulator (firmware is built with PlatformIO: pio run -t upload)
build/dyad_ghost: native/ghost.cpp src/dyad_engine.h
	mkdir -p build
	g++ -O2 -std=c++17 -Wall -Wno-format-truncation native/ghost.cpp -o $@
clean:
	rm -rf build
