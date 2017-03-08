#!/bin/sh

ANDROID_TEST_ROOT=tests/android
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

# loop and run each test case
for dir in ${TEST_DIRS[@]}; do
    SHELLJS="${REMOTE_TEST_DIR}/${dir}/shell.js"

    for tc in $(find ${MOZILLA_TEST_ROOT}/$dir -name '*.js'); do
        if [[ $tc == *browser.js ]]; then continue; fi
        if [[ $tc == *jsref.js ]]; then continue; fi
        if [[ $tc == *shell.js ]]; then continue; fi

        echo $tc

        # kill Native Shell
        adb shell am force-stop com.example.native_shell

        TMPFILE=$(mktemp -t jsclite-test-case)
        if [ $? -ne 0 ]; then
            echo "can't create temp file, exiting..."
            exit 1
        fi
        echo "$TMPFILE"
        rm "$TMPFILE"

        OUTPUT=$(basename "$TMPFILE")
        echo "$OUTPUT"

        # test Native Shell with -e shell shell.js -e test test/path.js -e output jslite-test-case.g7d94e39

        TCJS="${REMOTE_TEST_DIR}${tc#$MOZILLA_TEST_ROOT}"
        echo "$TCJS"

        adb shell am start -a android.intent.action.MAIN \
                -e shell "$SHELLJS" \
                -e test "$TCJS" \
                -e output "$REMOTE_TEST_DIR/$OUTPUT" \
                -n com.example.native_shell/com.example.nativeshell.NativeApp

        # sleep and wait for test case to write a file to sdcard

        TIMEOUT=0
        until adb pull "$REMOTE_TEST_DIR/$OUTPUT" "$TMPFILE" 2>&1 > /dev/null || [ $TIMEOUT -eq 10 ]; do
            echo "waiting for test to complete ($TIMEOUT)"
            sleep 1
            ((TIMEOUT++))
        done

        adb shell rm "$REMOTE_TEST_DIR/$OUTPUT"

        # if we didn't get the file
        if [ ! -e "$TMPFILE" ]; then
            #  note the failure
            echo "test timeout. continuing..."
            continue
        fi

        # did the test pass or fail or error
        # note the pass/fail/error

        # append output to test run file
        cat "$TMPFILE" >> "$ANDROID_TEST_ROOT/actual.txt"

        # insert blank line after test output
        echo "" >> "$ANDROID_TEST_ROOT/actual.txt"

        rm "$TMPFILE"

    done
done
