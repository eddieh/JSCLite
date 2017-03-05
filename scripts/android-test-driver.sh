#!/bin/sh

MOZILLA_TEST_ROOT=tests/mozilla
REMOTE_TEST_DIR=/sdcard/jsclite-tests
TEST_DIRS=(
    ecma
    ecma_2
    ecma_3
    js1_1
    js1_2
    js1_3
    js1_4
    js1_5
)

echo "remove old test dir ${REMOTE_TEST_DIR}"
adb shell rm -r $REMOTE_TEST_DIR

echo "create test dir ${REMOTE_TEST_DIR}"
adb shell mkdir -p $REMOTE_TEST_DIR

for dir in ${TEST_DIRS[@]}; do
    adb push ${MOZILLA_TEST_ROOT}/$dir ${REMOTE_TEST_DIR}/$dir
done

# maybe remove old Native Shell...

# install Native Shell

# launch Native Shell with -e shell shell.js -e test test/path.js
adb shell am start -a android.intent.action.MAIN \
    -e shell /sdcard/jsclite-tests/ecma/shell.js \
    -e test /sdcard/jsclite-tests/ecma/Array/15.4-1.js \
    -n com.example.native_shell/com.example.nativeshell.NativeApp
