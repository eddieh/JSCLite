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

void android_main(struct android_app *app) {
    struct engine engine;

    // Make sure glue isn't stripped.
    app_dummy();

    memset(&engine, 0, sizeof(engine));
    app->userData = &engine;
    app->onAppCmd = engine_handle_cmd;
    app->onInputEvent = engine_handle_input;
    engine.app = app;

    if (!engine.ctx) {
        engine.ctx = JSGlobalContextCreate(NULL);

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
            LOGI(">>>>>>>>>>>>>> shell_cstr = %s", shell_cstr);
        }

        if (test) {
            char *test_cstr = (*env)->GetStringUTFChars(env, test, NULL);
            LOGI(">>>>>>>>>>>>>> test_cstr = %s", test_cstr);
        }
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
