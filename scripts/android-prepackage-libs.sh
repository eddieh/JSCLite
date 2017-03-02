#!/bin/sh

PREPACKAGED_LIBS_DIR=build/Android.prepackaged-libs
ANDROID_ARCHS=(armeabi armeabi-v7a)

mkdir -p $PREPACKAGED_LIBS_DIR
cd $PREPACKAGED_LIBS_DIR

for arch in ${ANDROID_ARCHS[@]}; do
    mkdir -p $arch
    cp ../Android.$arch/libjs.so $arch/libjs.so
done
