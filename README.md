# JSCLite

Lightweight JavaScriptCore (JSC) for embedding in apps on non-Apple
platforms.

This minimalist variant of JavaScriptCore is great for embedding in
Android apps or in embedded systems. Typically JavaScriptCore is
prohibitively large and not easily embedded. This project aims to
create a minimalist variant that can be embedded without adding too
much bloat to your project.

<!-- Insert libjs.so size table each Android architecture. -->


## Prerequisites for the Host

CMake v2.6+, make, and a working compiler.


## Building for the Host

Compile for the host by running `make`.


## Running Test on the Host

Run the tests by running `make test`.


## Prerequisites for Android

In order to build for Android you must have the ability to build for
your host system. Compilation relies on a small program that must be
compiled which generates a hash table.

To build the library you need CMake v2.6+ and Android NDK r10e or
earlier.

To build the test app you need the Android SDK. Recent versions of the
SDK will require specific versions of the NDK. It is best to use the
NDK bundled with the SDK when building the test app.


## Building for Android

In order for CMake to use the correct version of the NDK you must set
an environment variable. Set `ANDROID_NDK` to the NDK. Once your
environment variable is set, you can build with the following make
invocation

    make android

This will build for the host and then build for Android. After you
have the host and the library compiling you can now simply build from
the Native Shell directory. The Native Shell is set up to
automatically build the library as needed.

See the `README.md` file in `tests/android/native-shell` for building
the Native Shell test app.


## Running Test on Android

Complete all the steps for compiling on the host, building for
Android, and building & installing the Native Shell.

With an Android device connected you over USB, install the Native Shell
APK, then use the following invocation

    scripts/android-test-driver.sh
