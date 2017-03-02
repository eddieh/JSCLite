package com.example.nativeshell;

import android.app.NativeActivity;

public class NativeApp extends NativeActivity {
    /* */
    static {
        System.loadLibrary("js");
    }
}
