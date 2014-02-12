/*
 * Copyright (C) 2009 The Android Open Source Project
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
 */

// OpenGL ES 2.0 code

#include <time.h>

#include "game.cpp"
#include "util.cpp"

extern "C" {
    JNIEXPORT void JNICALL Java_com_android_gunner_GunnerLib_init(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT void JNICALL Java_com_android_gunner_GunnerLib_step(JNIEnv * env, jobject obj);
};

Game game;

JNIEXPORT void JNICALL Java_com_android_gunner_GunnerLib_init(JNIEnv * env, jobject obj,  jint width, jint height)
{
    game.init((int)width, (int)height);
}

JNIEXPORT void JNICALL Java_com_android_gunner_GunnerLib_step(JNIEnv * env, jobject obj)
{
    static struct timespec time;
    struct timespec newTime;
    clock_gettime(CLOCK_REALTIME, &newTime);
    double dt = (newTime.tv_nsec - time.tv_nsec) / 1000000000.0;
    if (dt < 0) { dt += 1; }
    time = newTime;

    game.work(dt);
}
