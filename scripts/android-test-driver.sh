#!/bin/sh

usage () {
    echo "Usage: $0 [test1] [test2] [testN] [-f listoftests]" 1>&2
    exit 1
}

while getopts ":f:" opt; do
    case $opt in
        f)
            f=${OPTARG}
            ;;
        \?)
            usage
            ;;
    esac
done

function main () {
    if [ -n "$f" ]; then
        # -f file arguments: run all test in file
        echo "Run tests from ${f}"
    elif [ ${#@} -gt 0 ]; then
        # test1 test2 testN arguments: run each test
        echo "Run each of these tests:"
        run_test_case_set $@
    else
        # no arguments to script: run all tests
        echo "Run all tests!"
        run_all_tests
    fi
    report
}

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

# save start time
STARTTIME=$(date +'%s')

function install_tests() {
    echo "remove old test dir ${REMOTE_TEST_DIR}"
    adb shell rm -r $REMOTE_TEST_DIR

    echo "create test dir ${REMOTE_TEST_DIR}"
    adb shell mkdir -p $REMOTE_TEST_DIR

    for dir in ${TEST_DIRS[@]}; do
        adb push ${MOZILLA_TEST_ROOT}/$dir ${REMOTE_TEST_DIR}/$dir
    done
}

# TODO: we don't want to install the tests on every invocation
install_tests

# maybe remove old Native Shell...

# install Native Shell

# after installing grant the following permissions

# adb shell pm grant com.example.native_shell android.permission.WRITE_EXTERNAL_STORAGE
# adb shell pm grant com.example.native_shell android.permission.READ_EXTERNAL_STORAGE

if [ -e "$ANDROID_TEST_ROOT/actual.txt" ]; then
    rm "$ANDROID_TEST_ROOT/actual.txt"
fi

TESTCOUNT=0

declare -a FAILURES
FAILINDEX=0

declare -a TIMEOUTS
TIMEOUTINDEX=0

function run_test_case_set() {
    # $@ argument should be a set of test paths
    for tc in $@; do
        if [[ $tc == *browser.js ]]; then continue; fi
        if [[ $tc == *jsref.js ]]; then continue; fi
        if [[ $tc == *shell.js ]]; then continue; fi

        ((TESTCOUNT++))

        echo $tc

        # kill Native Shell
        adb shell am force-stop com.example.native_shell
        #sleep 5

        TMPFILE=$(mktemp -t jsclite-test-case)
        if [ $? -ne 0 ]; then
            echo "can't create temp file, exiting..."
            exit 1
        fi
        #echo "$TMPFILE"
        rm "$TMPFILE"

        OUTPUT=$(basename "$TMPFILE")
        #echo "$OUTPUT"

        # test Native Shell with -e shell shell.js -e test test/path.js -e output jslite-test-case.g7d94e39

        TCJS="${REMOTE_TEST_DIR}${tc#$MOZILLA_TEST_ROOT}"
        #echo "$TCJS"

        # guess shell.js file
        if [ -z $SHELLJS ]; then
           #echo "Guessing shell.js path"
           dir=$(cut -d/ -f3 <<<"$tc")
           SHELLJS="${REMOTE_TEST_DIR}/${dir}/shell.js"
        fi

        adb shell am start -a android.intent.action.MAIN \
            -e shell "$SHELLJS" \
            -e test "$TCJS" \
            -e output "$REMOTE_TEST_DIR/$OUTPUT" \
            -n com.example.native_shell/com.example.nativeshell.NativeApp #2>&1 > /dev/null
        # should check if the app started
        #sleep 5


        # If the file is open the following will return an empty file.
        # We need to also loop while the file is empty.
        # And need to timeout if just the same if the file remains empty for too long.


        # sleep and wait for test case to write a file to sdcard
        TIMEOUT=0
        #until adb pull "$REMOTE_TEST_DIR/$OUTPUT" "$TMPFILE" 2>&1 > /dev/null || [ $TIMEOUT -eq 10 ]; do
        until adb pull "$REMOTE_TEST_DIR/$OUTPUT" "$TMPFILE" || [ $TIMEOUT -eq 30 ]; do
            echo "waiting for test to open output file ($TIMEOUT)"
            sleep 1
            ((TIMEOUT++))
        done

        # if we didn't get the file
        echo "looking for temp file"
        if [ ! -e "$TMPFILE" ]; then
            #  note the failure
            echo "test timeout. continuing..."
            TIMEOUTS[TIMEOUTINDEX]=$tc
            ((TIMEOUTINDEX++))

            FAILURES[FAILINDEX]=$tc
            ((FAILINDEX++))

            adb shell rm "$REMOTE_TEST_DIR/$OUTPUT"
            continue
        fi

        echo "is file empty?"
        TIMEOUT=0
        until grep '[[:alnum:]]' "$TMPFILE" || [ $TIMEOUT -eq 60 ]; do
            echo "waiting for test to complete ($TIMEOUT)"
            adb pull "$REMOTE_TEST_DIR/$OUTPUT" "$TMPFILE"
            sleep 1
            ((TIMEOUT++))
        done

        if [ $(wc -c "$TMPFILE" | awk {'print $1'}) -lt 8 ]; then
            echo "test timeout."
            TIMEOUTS[TIMEOUTINDEX]=$tc
            ((TIMEOUTINDEX++))

            FAILURES[FAILINDEX]=$tc
            ((FAILINDEX++))

            adb shell rm "$REMOTE_TEST_DIR/$OUTPUT"
            rm "$TMPFILE"
            continue
        fi

        # did the test pass or fail or error
        # note the pass/fail/error
        echo "searching for 'FAILED!' in file"
        if grep 'FAILED!' "$TMPFILE"; then
            FAILURES[FAILINDEX]=$tc
            ((FAILINDEX++))
            echo "test failed"
        fi

        # append output to test run file
        echo "append results"
        cat "$TMPFILE" >> "$ANDROID_TEST_ROOT/actual.txt"

        # insert blank line after test output
        echo "" >> "$ANDROID_TEST_ROOT/actual.txt"

        echo "delete temp files"
        adb shell rm "$REMOTE_TEST_DIR/$OUTPUT"
        rm "$TMPFILE"
    done
}

function run_all_tests() {
    # loop and run each test case
    for dir in ${TEST_DIRS[@]}; do
        SHELLJS="${REMOTE_TEST_DIR}/${dir}/shell.js"
        run_test_case_set $(find ${MOZILLA_TEST_ROOT}/$dir -name '*.js')
    done
}

function report() {
    echo ""
    echo "$FAILINDEX failure(s) of $TESTCOUNT tests."
    echo ""

    for failure in ${FAILURES[@]}; do
        echo $failure
    done

    echo ""
    echo "$TIMEOUTINDEX failure(s) from timeouts."
    echo ""

    for to in ${TIMEOUTS[@]}; do
        echo $to
    done


    # compare with expected

    # new failures not in expected are tests recently broken
    # failues only in expected are tests recently fixed

    # print run time
    ENDTIME=$(date +'%s')
    DURATION=$(expr $ENDTIME - $STARTTIME)
    DURATION=$(expr $DURATION / 60)

    echo ""
    echo "$DURATION minute(s)"
}

main $@
