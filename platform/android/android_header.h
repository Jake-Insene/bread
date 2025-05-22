#pragma once

// GLES
#include <EGL/egl.h>
#include <EGL/eglext.h>

// Don't use prototypes
#define GL_GLES_PROTOTYPES 0
#include <GLES3/gl32.h>

// Android
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/set_abort_message.h>
#include <android_native_app_glue.h>
#include <android/native_window_jni.h>
#include <jni.h>

#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
