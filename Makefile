# this seems to be the only reliable way to get the directory this
# Makefile is in when building as a dependency through Gradle
PROJ_ROOT=$(strip $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))))

BUILD_BASE_DIR = build
BUILD_HOST_NAME := $(shell uname -s)
BUILD_HOST_ARCH := $(shell uname -m)
BUILD_HOST_DIR = $(BUILD_BASE_DIR)/$(BUILD_HOST_NAME).$(BUILD_HOST_ARCH)

CROSS_BUILD_CMAKE_FLAGS = -DIMPORT_PATH=$(PROJ_ROOT)/$(BUILD_HOST_DIR)

ANDROID_ARCHS = armeabi armeabi-v7a
ANDROID_BUILD_BASE_DIR = $(BUILD_BASE_DIR)/Android
ANDROID_CMAKE_FLAGS = $(CROSS_BUILD_CMAKE_FLAGS) \
	-DCMAKE_TOOLCHAIN_FILE="toolchains/android.cmake"

# By default just build for the host platform & architecture.
all: host

host:
	cmake -E make_directory $(BUILD_HOST_DIR) && \
	cmake -E chdir $(BUILD_HOST_DIR) cmake ../.. && \
	cmake --build $(BUILD_HOST_DIR)

# The host must be built before any cross compile builds. Cross
# compiling relies on a binary that must run on the host to generate
# some files.
android: host
	for arch in $(ANDROID_ARCHS); do \
		cmake -E make_directory $(ANDROID_BUILD_BASE_DIR).$$arch && \
		cmake -E chdir $(ANDROID_BUILD_BASE_DIR).$$arch cmake ../.. \
			-DANDROID_ARCH_ABI=$$arch \
			$(ANDROID_CMAKE_FLAGS) && \
		cmake --build $(ANDROID_BUILD_BASE_DIR).$$arch ; done
	sh scripts/android-prepackage-libs.sh

# By default just run the tests on the host.
test: test-host

test-host:
	cd tests/mozilla && SYMROOTS=../../$(BUILD_HOST_DIR) ./run-mozilla-tests

clean:
	rm -r build
