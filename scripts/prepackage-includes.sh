#!/bin/sh

PREPACKAGED_INCLUDES_DIR=build/include/JavaScriptCore
INCLUDES=(
  JSBase.h
  JSContextRef.h
  JSObjectRef.h
  JSStringRef.h
  JSValueRef.h
  JavaScriptCore.h
)

mkdir -p $PREPACKAGED_INCLUDES_DIR
cd $PREPACKAGED_INCLUDES_DIR

for inc in ${INCLUDES[@]}; do
    cp ../../../API/$inc .
done
