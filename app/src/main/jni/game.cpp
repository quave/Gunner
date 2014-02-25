#ifndef GAME_CPP
#define GAME_CPP

#include <jni.h>
#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <JNIHelper.h>
#include <android/input.h>
#include <vecmath.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <vector>

#include "util.cpp"
#include "node.cpp"
#include "shuttle.cpp"
#include "meteor.cpp"
#include "bullet.cpp"

using namespace ndk_helper;
using namespace std;

class Game {
    bool inited_;
    GLuint gProgram_;
    GLuint gaPositionHandle_;
    GLuint gaColorHandle_;

    GLuint loadShader(GLenum shaderType, const char* pSource);
    GLuint createProgram(const char* pVertexSource, const char* pFragmentSource);

    Mat4 perspective_;
    Mat4 view_;
    int width_;
    int height_;
    double time_;

    Shuttle* shuttle_;
    vector<Node*> scene_;
    //vector<int> deleted_;

    static const float fallSpeed = 0.2f;
    static const float bulletSpeed = 0.7f;
    static const float shuttleSpeed = 0.3f;
    static const int smallMeteors = 4;

    void updateMeteor(double dt, vector<Node*>::iterator nodeIt);
    void updateBullet(double dt, vector<Node*>::iterator nodeIt);

public:
    Game(int w, int h): inited_(false), time_(0) { init(w, h); };
    ~Game();
    bool init(int w, int h);
    void deInit() {};
    void work(double dt);
    void pause();
    void resume();
    void tap(float x, float y);
};

const char gVertexShader[] =
    "attribute vec4 aPosition;\n"
    "attribute vec4 aColor;\n"
    "varying vec4 vColor;\n"
    "void main() {\n"
    "  vColor = aColor;\n"
    "  gl_Position = aPosition;\n"
    "}\n";

const char gFragmentShader[] =
    "precision mediump float;\n"
    "varying vec4 vColor;\n"
    "void main() {\n"
    "  gl_FragColor = vColor;\n"
    "}\n";

GLuint Game::loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (!shader) { return shader; }

    glShaderSource(shader, 1, &pSource, NULL);
    glCompileShader(shader);
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled) { return shader; }

    GLint infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
    if (!infoLen) { return shader; }

    char* buf = (char*) malloc(infoLen);
    if (buf) {
        glGetShaderInfoLog(shader, infoLen, NULL, buf);
        LOGE("Could not compile shader %d:\n%s\n", shaderType, buf);
        free(buf);
    }
    glDeleteShader(shader);
    shader = 0;

    return shader;
}

GLuint Game::createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) { return 0; }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) { return 0; }

    GLuint program = glCreateProgram();
    if (!program) { return program; }

    glAttachShader(program, vertexShader);
    checkGlError("glAttachShader");
    glAttachShader(program, pixelShader);
    checkGlError("glAttachShader");
    glLinkProgram(program);
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (linkStatus == GL_TRUE) { return program; }

    GLint bufLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);

    if (bufLength) {
        char* buf = (char*) malloc(bufLength);
        if (buf) {
            glGetProgramInfoLog(program, bufLength, NULL, buf);
            LOGE("Could not link program:\n%s\n", buf);
            free(buf);
        }
    }
    glDeleteProgram(program);
    program = 0;

    return program;
}

bool Game::init(int w, int h) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    width_ = w;
    height_ = h;

    LOGI("setupGraphics(%d, %d)", w, h);
    gProgram_ = createProgram(gVertexShader, gFragmentShader);
    if (!gProgram_) {
        LOGE("Could not create program.");
        return false;
    }
    gaPositionHandle_ = glGetAttribLocation(gProgram_, "aPosition");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"aPosition\") = %d\n", gaPositionHandle_);
    gaColorHandle_ = glGetAttribLocation(gProgram_, "aColor");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"gaColorHandle_\") = %d\n", gaColorHandle_);

    glViewport(0, 0, w, h);
    checkGlError("glViewport");

    struct timeval now;
    gettimeofday(&now, NULL);
    srand(now.tv_usec);

    if (!inited_) {
        shuttle_ = new Shuttle(width_, height_);
        scene_.push_back(shuttle_);

        for (int i = 0; i < 2; ++i) {
            scene_.push_back(new Meteor(width_, height_));
        }

        inited_ = true;
    }

    return true;
}

void Game::pause() { LOGI("Game::pause"); }
void Game::resume() { LOGI("Game::resume"); }

void Game::tap(float x, float y) {
    Bullet* bullet = new Bullet(width_, height_);
    bullet->translate(shuttle_->getX(), 0.0f);
    scene_.push_back(bullet);

    float dx = x - shuttle_->getX();
    dx = copysignf(1.0, dx) * fmin(shuttleSpeed, abs(dx));

    shuttle_->translate(dx, 0.0f);
}

void Game::work(double dt) {
    time_ += dt;

    glClearColor(0.2353f, 0.2471f, 0.2549f, 1.0f);
    checkGlError("glClearColor");
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");

    glUseProgram(gProgram_);
    checkGlError("glUseProgram");

    glLineWidth(2.0f);

    for (vector<Node*>::iterator node = scene_.begin(); node < scene_.end(); ++node) {
        if (*node == NULL) {
            //scene_.erase(node);
            continue;
        }

        (*node)->draw(dt, gaPositionHandle_, gaColorHandle_);

        if ((*node)->getType() == METEOR) {
            updateMeteor(dt, node);
        }

        if ((*node)->getType() == BULLET) {
            updateBullet(dt, node);
        }
    }
}

void Game::updateMeteor(double dt, vector<Node*>::iterator nodeIt) {
    Meteor* meteor = (Meteor*) (*nodeIt);
    meteor->translate(0.0f, -(dt * fallSpeed));
    meteor->rotate(0.1f);

    if (meteor->isOut()) {
        //deleted_.push_back(nodeIt - scene_.begin());
        /*scene_.erase(nodeIt);
        delete meteor;*/

        /*Meteor* newMeteor = new Meteor(width_, height_);
        scene_.push_back(newMeteor);*/
    }
}

void Game::updateBullet(double dt, vector<Node*>::iterator nodeIt) {
    Bullet* bullet = (Bullet*) (*nodeIt);
    bullet->translate(0.0f, dt * bulletSpeed);

    if (bullet->isOut()) {
        scene_.erase(nodeIt);
        delete bullet;
    }

    /*bool collide = false;
    for (vector<Node*>::iterator node = scene_.begin(); node < scene_.end(); ++node) {
        if ((*node)->getType() == METEOR && bullet->isIntersect(*node)) {
            collide = true;

            Meteor* meteor = (Meteor*) (*node);
            float x = meteor->getX();
            float y = meteor->getY();
            //delete meteor;
            //*node = NULL;

            for (int i = 0; i < smallMeteors; ++i) {
                Meteor* smallMeteor = new Meteor(width_, height_);
                smallMeteor->scale(0.3f, 0.3f);
                smallMeteor->translate(x, y);
                scene_.push_back(smallMeteor);
            }
        }
    }

    if (collide) {
        delete bullet;
        *nodeIt = NULL;
    }*/
}

Game::~Game() {
    for (vector<Node*>::iterator node = scene_.begin(); node < scene_.end(); ++node) {
        scene_.erase(node);
        delete (*node);
    }
}

#endif