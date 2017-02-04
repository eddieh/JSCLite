#!/bin/sh

export SRCROOT=$1

mkdir -p DerivedSources/JavaScriptCore
cd DerivedSources/JavaScriptCore

make -f $SRCROOT/DerivedSources.make JavaScriptCore=$SRCROOT BUILT_PRODUCTS_DIR=$SRCROOT/build
