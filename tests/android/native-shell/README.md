Native Shell
============

Native shell for testing JSC on Android.


Prerequisite
============

First you must be able to build JSCLite for
Android. From JSCLite's root directory running `make android` must
succeed.

Once you can successfully build JSCLite then you can build the Native
Shell.


Building
========

Create a `local.properties` file in this directory. The file must
contain the keys `sdk.dir` and `ndk.dir`. Point these at the
appropriate locations on your machine.

Note: the NDK version used for building JSCLite and for building the
Native Shell can differ.

When building this app, use the NDK bundled with your SDK. The easiest
way to accomplish this given the following `sdk.dir` key and value

    sdk.dir=/Users/username/android-sdk_r24.4.1-macosx

is simply to append `/ndk-bundle` to the `sdk.dir` path value such
that its value is as follows

    ndk.dir=/Users/username/android-sdk_r24.4.1-macosx/ndk-bundle

This will not interfere with the settings for building JSCLite.

Once a proper `local.properties` file is in place you can build and
install the APK using Gradle.

    ./gradlew installDebug


Running Tests
=============

By itself this app does nothing. In order to run the Mozilla
JavaScript test suite use the `android-test-driver.sh` test driver
script located in the `JSCLite/scripts` directory. Assuming an Android
device is connected over USB and the Native Shell has been installed
on the device the following invocation from JSCLite's root directory
will run the entire test suite

    scripts/android-test-driver.sh
