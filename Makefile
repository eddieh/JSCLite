all: config
	cd build && make

build:
	mkdir -p build

config: build
	cd build && cmake ..

clean:
	rm -r build
