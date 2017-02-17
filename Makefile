BUILD_BASE_DIR = build
BUILD_HOST_NAME := $(shell uname -s)
BUILD_HOST_ARCH := $(shell uname -m)
BUILD_HOST_DIR = $(BUILD_BASE_DIR)/$(BUILD_HOST_NAME).$(BUILD_HOST_ARCH)

CROSS_BUILD_CMAKE_FLAGS = -DIMPORT_PATH=${PWD}/$(BUILD_HOST_DIR)

ANDROID_ARCHS = armeabi armeabi-v7a
ANDROID_BUILD_BASE_DIR = $(BUILD_BASE_DIR)/Android
ANDROID_CMAKE_FLAGS = $(CROSS_BUILD_CMAKE_FLAGS) \
	-DCMAKE_TOOLCHAIN_FILE="toolchains/android.cmake"

# by default just build for the hose platform & architecture
all: host

host:
	cmake -E make_directory $(BUILD_HOST_DIR) && \
	cmake -E chdir $(BUILD_HOST_DIR) cmake ../.. && \
	cmake --build $(BUILD_HOST_DIR)

android: host
	for arch in $(ANDROID_ARCHS); do \
		cmake -E make_directory $(ANDROID_BUILD_BASE_DIR).$$arch && \
		cmake -E chdir $(ANDROID_BUILD_BASE_DIR).$$arch cmake ../.. \
			-DANDROID_ARCH_ABI=$$arch \
			$(ANDROID_CMAKE_FLAGS) && \
		cmake --build $(ANDROID_BUILD_BASE_DIR).$$arch ; done

test:
	cd tests/mozilla && SYMROOTS=../../$(BUILD_HOST_DIR) ./run-mozilla-tests

clean:
	rm -r build
