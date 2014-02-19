#ifndef UTIL_CPP
#define UTIL_CPP

#define LOG_TAG "gunner"
#include <JNIHelper.h>
#include <android/log.h>
#include <GLES2/gl2.h>

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

#endif