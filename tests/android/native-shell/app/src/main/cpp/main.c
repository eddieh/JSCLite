/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <android_native_app_glue.h>
#include <android/log.h>

#include <JavaScriptCore/JavaScriptCore.h>

#include <stdio.h>
#include <string.h>

#define LOG_TAG "native-shell"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

/* Set to 1 to enable debug log traces. */
#define DEBUG 0

struct engine {
    struct android_app *app;
    JSGlobalContextRef ctx;
    int animating;
    char *shell;
    char *test;
};

static void engine_draw_frame(struct engine *engine) {
    if (engine->app->window == NULL)
        return;

    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(engine->app->window, &buffer, NULL) < 0) {
        LOGW("Unable to lock window buffer");
        return;
    }

    // if we did any drawing it would be here

    ANativeWindow_unlockAndPost(engine->app->window);
}

static void engine_term_display(struct engine *engine) {
    engine->animating = 0;
}

static int32_t engine_handle_input(struct android_app *app, AInputEvent *event) {
    struct engine *engine = (struct engine *)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->animating = 1;
        return 1;
    } else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
        LOGI("Key event: action=%d keyCode=%d metaState=0x%x",
             AKeyEvent_getAction(event),
             AKeyEvent_getKeyCode(event),
             AKeyEvent_getMetaState(event));
    }
    return 0;
}

static void engine_handle_cmd(struct android_app *app, int32_t cmd) {
    static int32_t format = WINDOW_FORMAT_RGB_565;
    struct engine *engine = (struct engine *)app->userData;
    switch (cmd) {
    case APP_CMD_INIT_WINDOW:
        if (engine->app->window != NULL) {
            format = ANativeWindow_getFormat(app->window);
            ANativeWindow_setBuffersGeometry(app->window,
                                             ANativeWindow_getWidth(app->window),
                                             ANativeWindow_getHeight(app->window),
                                             WINDOW_FORMAT_RGB_565);
            engine_draw_frame(engine);
        }
        break;
    case APP_CMD_TERM_WINDOW:
        engine_term_display(engine);
        ANativeWindow_setBuffersGeometry(app->window,
                                         ANativeWindow_getWidth(app->window),
                                         ANativeWindow_getHeight(app->window),
                                         format);
        break;
    case APP_CMD_LOST_FOCUS:
        engine->animating = 0;
        engine_draw_frame(engine);
        break;
    }
}

void init_test_case(struct android_app *app) {
    struct engine *engine = (struct engine *)app->userData;

    // Java: String shell = getIntent().getStringExtra("shell");
    // Java: String test = getIntent().getStringExtra("test");

    /* TODO: check for exceptions */

    ANativeActivity *activity = app->activity;
    JavaVM *vm = activity->vm;
    JNIEnv *env;

    (*vm)->AttachCurrentThread(vm, &env, NULL);

    jclass activityClass = (*env)->FindClass(env, "android/app/Activity");
    jclass intentClass = (*env)->FindClass(env, "android/content/Intent");

    jmethodID getIntent = (*env)->GetMethodID(
        env, activityClass, "getIntent", "()Landroid/content/Intent;");

    jmethodID getStringExtra = (*env)->GetMethodID(
        env, intentClass, "getStringExtra",
        "(Ljava/lang/String;)Ljava/lang/String;");

    jobject intentObj = (*env)->CallObjectMethod(
        env, activity->clazz /* <- bad name for an instance */, getIntent);

    jstring shell = (*env)->CallObjectMethod(
        env, intentObj, getStringExtra, (*env)->NewStringUTF(env, "shell"));

    jstring test = (*env)->CallObjectMethod(
        env, intentObj, getStringExtra, (*env)->NewStringUTF(env, "test"));

    if (shell) {
        char *shell_cstr = (*env)->GetStringUTFChars(env, shell, NULL);
        size_t s = strlen(shell_cstr);
        //size_t s = (size_t)(*env)->GetStringLength(env, shell);


        engine->shell = malloc(s + 1);
        memcpy(engine->shell, shell_cstr, s);
        engine->shell[s] = 0;

        LOGI(">>>>>>>>>>>>>> shell = %s", engine->shell);

        (*env)->ReleaseStringUTFChars(env, shell, shell_cstr);
    }

    if (test) {
        char *test_cstr = (*env)->GetStringUTFChars(env, test, NULL);
        size_t s = strlen(test_cstr);
        //size_t s = (size_t)(*env)->GetStringLength(env, test);

        engine->test = malloc(s + 1);
        memcpy(engine->test, test_cstr, s);
        engine->test[s] = 0;

        LOGI(">>>>>>>>>>>>>> test = %s", engine->test);

        (*env)->ReleaseStringUTFChars(env, test, test_cstr);
    }
}

void handle_exception(JSContextRef ctx, JSValueRef exception) {
    JSValueRef local_except = NULL;
    JSValueRef message, line, sourceId, name;
    JSStringRef message_js_str, name_js_str;
    JSObjectRef except_obj;
    char *message_str, *name_str;
    size_t message_len, name_len;
    double line_num;

    LOGI("* Exception:\n");

    except_obj = JSValueToObject(ctx, exception, &local_except);
    if (local_except != NULL) {
        LOGI("  couldn't coerce 'exception' value to object\n");
        goto error;
    }

    message = JSObjectGetProperty(ctx, except_obj, JSStringCreateWithUTF8CString("message"), &local_except);
    if (local_except != NULL) {
        LOGI("  couldn't get property 'messager' from 'exception' object\n");
        goto error;
    }

    message_js_str = JSValueToStringCopy(ctx, message, &local_except);
    if (local_except != NULL) {
        LOGI("  couldn't get string copy from 'message'\n");
        goto error;
    }

    message_len = JSStringGetLength(message_js_str) + 1;
    message_str = (char*)malloc(sizeof(char) * message_len);
    JSStringGetUTF8CString(message_js_str, message_str, message_len);

    line = JSObjectGetProperty(ctx, except_obj, JSStringCreateWithUTF8CString("line"), &local_except);
    if (local_except != NULL) {
        LOGI("  couldn't get property 'line' from 'exception' object\n");
        goto error;
    }

    line_num = JSValueToNumber(ctx, line, &local_except);
    if (local_except != NULL) {
        LOGI("  couldn't coerce 'line' value to number\n");
        goto error;
    }

    // TODO: handle sourceId
    // sourceId = JSObjectGetProperty(ctx, except_obj, JSStringCreateWithUTF8CString("sourceId"), &local_except);
    // if (local_exept != NULL)
    //   goto error;

    name = JSObjectGetProperty(ctx, except_obj, JSStringCreateWithUTF8CString("name"), &local_except);
    if (local_except != NULL) {
        LOGI("  couldn't get property 'name' from 'exception' object\n");
        goto error;
    }

    name_js_str = JSValueToStringCopy(ctx, name, &local_except);
    if (local_except != NULL) {
        LOGI("  couldn't get string copy from 'name'\n");
        goto error;
    }

    name_len = JSStringGetLength(name_js_str) + 1;
    name_str = (char*)malloc(sizeof(char) * name_len);
    JSStringGetUTF8CString(name_js_str, name_str, name_len);

    LOGI("  name: %s\n", name_str);
    LOGI("  message: %s\n", message_str);
    LOGI("  line: %G\n", line_num);

    goto cleanup;

error:
    LOGI("  exception while handling exception\n");
cleanup:
    if (message_js_str)
        JSStringRelease(message_js_str);

    if (name_js_str)
        JSStringRelease(name_js_str);

    if (message_str)
        free(message_str);

    if (name_str)
        free(name_str);
}

JSValueRef print_native(
    JSContextRef ctx, JSObjectRef fn, JSObjectRef this_obj,
    size_t argc, const JSValueRef argv[], JSValueRef* except)
{
    JSValueRef exception = NULL;

    if (argc <= 0)
        return JSValueMakeUndefined(ctx);

    for (int i = 0; i < argc; i++) {
        char* cstr;
        JSStringRef jsstr = JSValueToStringCopy(ctx, argv[i], &exception);
        // TODO: handle exception
        size_t cstr_len = JSStringGetLength(jsstr) + 1;
        cstr = (char*)malloc(sizeof(char) * cstr_len);
        JSStringGetUTF8CString(jsstr, cstr, cstr_len);

        LOGI("%s", cstr);

        JSStringRelease(jsstr);
        free(cstr);
    }

    // must return something
    return JSValueMakeUndefined(ctx);
}

JSValueRef noop_native(
    JSContextRef ctx, JSObjectRef fn, JSObjectRef this_obj,
    size_t argc, const JSValueRef argv[], JSValueRef* except)
{
    // must return something
    return JSValueMakeUndefined(ctx);
}

void addJSFunction(JSGlobalContextRef ctx, char *name, JSObjectCallAsFunctionCallback cb) {
    JSValueRef exception = NULL;
    JSObjectRef global = JSContextGetGlobalObject(ctx);
    JSStringRef fnName = JSStringCreateWithUTF8CString(name);
    JSObjectRef fn = JSObjectMakeFunctionWithCallback(ctx, fnName, cb);
    JSPropertyAttributes builtinAttrs = kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete;
    JSObjectSetProperty(ctx, global, fnName, (JSValueRef)fn, builtinAttrs, &exception);
    JSStringRelease(fnName);

    if (exception) {
        handle_exception(ctx, exception);
        return;
    }
}

char *readFile(const char *fileName)
{
    char* buffer;

    int buffer_size = 0;
    int buffer_capacity = 1024;
    buffer = (char*)malloc(buffer_capacity);

    FILE* f = fopen(fileName, "r");
    if (!f) {
        LOGI("Could not open file: %s\n", fileName);
        return 0;
    }

    while (!feof(f) && !ferror(f)) {
        buffer_size += fread(buffer + buffer_size, 1, buffer_capacity - buffer_size, f);
        if (buffer_size == buffer_capacity) { // guarantees space for trailing '\0'
            buffer_capacity *= 2;
            buffer = (char*)realloc(buffer, buffer_capacity);
            //assert(buffer);
        }

        //assert(buffer_size < buffer_capacity);
    }
    fclose(f);
    buffer[buffer_size] = '\0';

    return buffer;
}


/* Note:
 * In a proper app this would run asynchronously with respect to the
 * event loop thread.
 */
void run_test_case(struct android_app *app) {
    struct engine *engine = (struct engine *)app->userData;
    JSValueRef exception = NULL;

    engine->ctx = JSGlobalContextCreate(NULL);

    // print, quit, gc, version

    addJSFunction(engine->ctx, "print", print_native);
    addJSFunction(engine->ctx, "quit", noop_native);
    addJSFunction(engine->ctx, "gc", noop_native);
    addJSFunction(engine->ctx, "version", noop_native);

    char *shellScript = readFile(engine->shell);
    char *testScript = readFile(engine->test);

    JSStringRef shellSrc = JSStringCreateWithUTF8CString(shellScript);
    JSStringRef testSrc = JSStringCreateWithUTF8CString(testScript);

    JSEvaluateScript(engine->ctx, shellSrc, NULL, NULL, 0, &exception);
    if (exception) {
        handle_exception(engine->ctx, exception);
        return;
    }

    JSEvaluateScript(engine->ctx, testSrc, NULL, NULL, 0, &exception);
    if (exception) {
        handle_exception(engine->ctx, exception);
        return;
    }

    JSStringRelease(shellSrc);
    JSStringRelease(testSrc);
}

void android_main(struct android_app *app) {
    static int init;

    struct engine engine;

    // Make sure glue isn't stripped.
    app_dummy();

    memset(&engine, 0, sizeof(engine));
    app->userData = &engine;
    app->onAppCmd = engine_handle_cmd;
    app->onInputEvent = engine_handle_input;
    engine.app = app;

    if (!init) {
        init_test_case(app);
        run_test_case(app);
        init = 1;
    }

    // loop waiting for stuff to do.

    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source *source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
                                        (void **)&source)) >= 0) {

            // Process this event.
            if (source != NULL)
                source->process(app, source);

            // Check if we are exiting.
            if (app->destroyRequested != 0) {
                LOGI("Engine thread destroy requested!");
                engine_term_display(&engine);
                return;
            }
        }

        if (engine.animating)
            engine_draw_frame(&engine);
    }
}
