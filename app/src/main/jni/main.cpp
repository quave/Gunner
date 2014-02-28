/*
 * Copyright 2013 The Android Open Source Project
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

//--------------------------------------------------------------------------------
// Include files
//--------------------------------------------------------------------------------
#include <jni.h>
#include <errno.h>

#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/native_window_jni.h>
#include <cpu-features.h>
#include <NDKHelper.h>
#include <string>

#include "util.cpp"
#include "game.cpp"

using namespace std;

//-------------------------------------------------------------------------
//Preprocessor
//-------------------------------------------------------------------------
#define HELPER_CLASS_NAME "com/android/gunner/NDKHelper" //Class name of helper function
//-------------------------------------------------------------------------
//Shared state for our app.
//-------------------------------------------------------------------------
struct android_app;
class Engine
{
    Game* game_;

    ndk_helper::GLContext* glContext_;

    bool initializedResources_;
    bool hasFocus_;

    ndk_helper::DragDetector dragDetector_;
    ndk_helper::PerfMonitor monitor_;
    double time_;

    android_app* app_;

    void transformPosition( ndk_helper::Vec2& vec );

public:
    static void handleCmd( struct android_app* app, int32_t cmd );
    static int32_t handleInput( android_app* app, AInputEvent* event );

    Engine();

    void setState( android_app* state );
    int initDisplay();
    void drawFrame();
    void termDisplay();
    void trimMemory();
    bool isReady();

    void showUI();
    void showScore(int score);
    void showCenterText(string text);
};

//-------------------------------------------------------------------------
//Ctor
//-------------------------------------------------------------------------
Engine::Engine() :
                initializedResources_( false ),
                hasFocus_( false ),
                app_( NULL ),
                time_ ( 0 )
{
    glContext_ = ndk_helper::GLContext::GetInstance();
}

void Engine::showUI()
{
    JNIEnv *jni;
    app_->activity->vm->AttachCurrentThread( &jni, NULL );

    //Default class retrieval
    jclass clazz = jni->GetObjectClass( app_->activity->clazz );
    jmethodID methodID = jni->GetMethodID( clazz, "showUI", "()V" );
    jni->CallVoidMethod( app_->activity->clazz, methodID );

    app_->activity->vm->DetachCurrentThread();
    return;
}

void Engine::showScore(int score) {
    JNIEnv *jni;
    app_->activity->vm->AttachCurrentThread( &jni, NULL );

    //Default class retrieval
    jclass clazz = jni->GetObjectClass( app_->activity->clazz );
    jmethodID methodID = jni->GetMethodID( clazz, "showScore", "(I)V" );
    jni->CallVoidMethod( app_->activity->clazz, methodID, score);

    app_->activity->vm->DetachCurrentThread();
    return;
}

void Engine::showCenterText(string text) {
    JNIEnv *jni;
    app_->activity->vm->AttachCurrentThread( &jni, NULL );

    jstring jtext = jni->NewStringUTF(text.c_str());
    //Default class retrieval
    jclass clazz = jni->GetObjectClass( app_->activity->clazz );
    jmethodID methodID = jni->GetMethodID( clazz, "showCenterText", "(Ljava/lang/String;)V" );
    jni->CallVoidMethod( app_->activity->clazz, methodID, jtext);

    app_->activity->vm->DetachCurrentThread();
    return;
}

/**
 * Initialize an EGL context for the current display.
 */
int Engine::initDisplay()
{
    if( !initializedResources_ )
    {
        glContext_->Init( app_->window );
        initializedResources_ = true;
    }
    else
    {
        // initialize OpenGL ES and EGL
        if( EGL_SUCCESS != glContext_->Resume( app_->window ) )
        {
            LOGI("GLContext Resume failed");
        }
    }

    showUI();

    game_ = new Game(glContext_->GetScreenWidth(), glContext_->GetScreenHeight());

    LOGI("end init");

    return 0;
}

/**
 * Just the current frame in the display.
 */
void Engine::drawFrame()
{
    double newTime = monitor_.GetCurrentTime();
    double dt = time_ == 0 ? 0 : newTime - time_;
    time_ = newTime;

    game_->work(dt);
    showScore(game_->getScore());

    // Swap
    if( EGL_SUCCESS != glContext_->Swap() )
    {
        LOGI("GLContext::Swap failed");
    }

    if (game_->isOver()) {
        showCenterText(game_->getGameOverText());
    }
}

/**
 * Tear down the EGL context currently associated with the display.
 */
void Engine::termDisplay()
{
    glContext_->Suspend();
}

void Engine::trimMemory()
{
    LOGI( "Trimming memory" );
    glContext_->Invalidate();
}
/**
 * Process the next input event.
 */
int32_t Engine::handleInput( android_app* app, AInputEvent* event )
{
    Engine* eng = (Engine*) app->userData;
    if( AInputEvent_getType( event ) != AINPUT_EVENT_TYPE_MOTION )
    {
        return 0;
    }

    ndk_helper::GESTURE_STATE dragState = eng->dragDetector_.Detect( event );

    if( dragState == ndk_helper::GESTURE_STATE_START )
    {
        ndk_helper::Vec2 v;
        eng->dragDetector_.GetPointer( v );
        eng->transformPosition(v);

        float x, y;
        v.Value(x, y);

        eng->game_->tap(x, y);
    }

    return 1;
}

/**
 * Process the next main command.
 */
void Engine::handleCmd( struct android_app* app, int32_t cmd )
{
    Engine* eng = (Engine*) app->userData;
    switch( cmd )
    {
    case APP_CMD_SAVE_STATE:
        LOGI("APP_CMD_SAVE_STATE");
        break;
    case APP_CMD_INIT_WINDOW:
        LOGI("APP_CMD_INIT_WINDOW");
        // The window is being shown, get it ready.
        if( app->window != NULL )
        {
            eng->initDisplay();
            eng->drawFrame();
        }
        break;
    case APP_CMD_TERM_WINDOW:
        LOGI("APP_CMD_TERM_WINDOW");
        // The window is being hidden or closed, clean it up.
        eng->termDisplay();
        eng->hasFocus_ = false;
        break;
    case APP_CMD_STOP:
        LOGI("APP_CMD_STOP");
        break;
    case APP_CMD_GAINED_FOCUS:
        LOGI("APP_CMD_GAINED_FOCUS");
        //Start animation
        eng->hasFocus_ = true;
        break;
    case APP_CMD_LOST_FOCUS:
        LOGI("APP_CMD_LOST_FOCUS");
        // Also stop animating.
        eng->hasFocus_ = false;
        eng->drawFrame();
        break;
    case APP_CMD_LOW_MEMORY:
        LOGI("APP_CMD_LOW_MEMORY");
        //Free up GL resources
        eng->trimMemory();
        break;
    }
}

//-------------------------------------------------------------------------
//Misc
//-------------------------------------------------------------------------
void Engine::setState( android_app* state )
{
    app_ = state;
    dragDetector_.SetConfiguration( app_->config );
}

bool Engine::isReady()
{
    if( hasFocus_ )
        return true;

    return false;
}

void Engine::transformPosition( ndk_helper::Vec2& vec )
{
    vec = ndk_helper::Vec2( 2.0f, 2.0f ) * vec
            / ndk_helper::Vec2( glContext_->GetScreenWidth(), glContext_->GetScreenHeight() )
            - ndk_helper::Vec2( 1.f, 1.f );
}

Engine g_engine;

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main( android_app* state )
{
    app_dummy();

    g_engine.setState( state );

    //Init helper functions
    ndk_helper::JNIHelper::Init( state->activity, HELPER_CLASS_NAME );

    state->userData = &g_engine;
    state->onAppCmd = Engine::handleCmd;
    state->onInputEvent = Engine::handleInput;

#ifdef USE_NDK_PROFILER
    monstartup("libgunner.so");
#endif

    // loop waiting for stuff to do.
    while( 1 )
    {
        // Read all pending events.
        int id;
        int events;
        android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while( (id = ALooper_pollAll( g_engine.isReady() ? 0 : -1, NULL, &events, (void**) &source ))
                >= 0 )
        {
            // Process this event.
            if( source != NULL )
                source->process( state, source );

            // Check if we are exiting.
            if( state->destroyRequested != 0 )
            {
                g_engine.termDisplay();
                return;
            }
        }

        if( g_engine.isReady() )
        {
            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            g_engine.drawFrame();
        }


    }
}
