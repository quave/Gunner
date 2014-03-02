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
#include <algorithm>
#include <string>
#include <sstream>

#include "util.cpp"
#include "node.cpp"
#include "shuttle.cpp"
#include "meteor.cpp"
#include "smallMeteor.cpp"
#include "bullet.cpp"

using namespace ndk_helper;
using namespace std;

class Game {
    GLuint gProgram_;
    GLuint gaPositionHandle_;
    GLuint gaColorHandle_;
    GLuint guVeiwProjHandle_;

    GLuint loadShader(GLenum shaderType, const char* pSource);
    GLuint createProgram(const char* pVertexSource, const char* pFragmentSource);

    Mat4 mProj_;
    int width_;
    int height_;
    float smallMeteorX_;
    float smallMeteorY_;
    int score_;
    bool isOver_;

    Shuttle* shuttle_;
    vector<Node*> scene_;
    vector<int> deleted_;

    static const int smallMeteors = 4;

    void updateMeteor(double dt, vector<Node*>::iterator nodeIt);
    void updateBullet(double dt, vector<Node*>::iterator nodeIt);

public:
    Game(int w, int h);
    ~Game();
    void work(double dt);
    void tap(float x, float y);
    bool isOver() { return isOver_; }
    string getGameOverText();
    int getScore() { return score_; }
};

const char gVertexShader[] =
    "uniform highp mat4 uViewProj;\n"
    "attribute vec2 aPosition;\n"
    "attribute vec4 aColor;\n"
    "varying vec4 vColor;\n"
    "void main() {\n"
    "  highp vec4 p = vec4(aPosition, 0, 1);\n"
    "  vColor = aColor;\n"
    "  gl_Position = uViewProj * p;\n"
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

Game::Game(int w, int h)
    : score_(0), isOver_(false), width_(w), height_(h),
    smallMeteorX_(0.0f), smallMeteorY_(0.0f)
{
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    // Init GLES
    LOGI("setupGraphics(%d, %d)", width_, height_);
    gProgram_ = createProgram(gVertexShader, gFragmentShader);
    if (!gProgram_) {
        LOGE("Could not create program.");
        return;
    }
    gaPositionHandle_ = glGetAttribLocation(gProgram_, "aPosition");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"aPosition\") = %d\n", gaPositionHandle_);
    gaColorHandle_ = glGetAttribLocation(gProgram_, "aColor");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"gaColorHandle_\") = %d\n", gaColorHandle_);
    guVeiwProjHandle_ = glGetUniformLocation(gProgram_, "uViewProj");
    checkGlError("glGetUniformLocation");
    LOGI("glGetUniformLocation(\"guVeiwProjHandle_\") = %d\n", guVeiwProjHandle_);

    glViewport(0, 0, w, h);
    checkGlError("glViewport");

    float aspect = (float) h / (float) w;
    float ortho[16] = { aspect, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f};
    mProj_ = Mat4((float*)&ortho);

    // Init random generator
    struct timeval now;
    gettimeofday(&now, NULL);
    srand(now.tv_usec);

    // Init scene objects
    shuttle_ = new Shuttle();
    scene_.push_back(shuttle_);
}

void Game::tap(float x, float y) {
    Bullet* bullet = new Bullet();
    bullet->translate(shuttle_->getX(), 0.0f);
    scene_.push_back(bullet);

    float dx = x - shuttle_->getX();
    dx = copysignf(1.0, dx) * fmin(shuttle_->getSpeed(), abs(dx));

    shuttle_->translate(dx, 0.0f);
}

void Game::work(double dt) {
    dt = fmin(dt, 1.0f);

    // Clear some buffers
    glClearColor(0.2353f, 0.2471f, 0.2549f, 1.0f);
    checkGlError("glClearColor");
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");
    // Use some programs
    glUseProgram(gProgram_);
    checkGlError("glUseProgram");

    glLineWidth(2.0f);

    // Randomly generate meteors at approximate rate one per second
    if ( ((float)rand() / RAND_MAX) < dt ) {
        Meteor* meteor = new Meteor();
        float sky = (float) width_ / (float)height_;
        float x = ((float)rand() / RAND_MAX) * sky  - sky / 2;
        meteor->translate(x, 1.0f);
        meteor->updateXSpeed();
        scene_.push_back(meteor);
    }

    // Render scene loop
    for (vector<Node*>::iterator node = scene_.begin(); node < scene_.end(); ++node) {
        // Let's draw it
        (*node)->draw(dt, gaPositionHandle_, gaColorHandle_, guVeiwProjHandle_, mProj_);

        enum NodeType type = (*node)->getType();

        // If it is a meteor than update it's position and stuff
        if (type == METEOR || type == SMALL_METEOR) {
            updateMeteor(dt, node);
        }

        // Do the same for bullet
        if (type == BULLET) {
            updateBullet(dt, node);
        }
    }

    if (!deleted_.empty()) {
        // Sort "deleted" array to be able remove elements in right order
        sort(deleted_.begin(), deleted_.end());
        // Remove duplicates
        deleted_.erase( unique( deleted_.begin(), deleted_.end() ), deleted_.end() );
    }
    // Remove elements in backwards order
    for (int i = deleted_.size() - 1; i >= 0; --i) {
        int index = deleted_[i];
        delete scene_[index];
        scene_.erase(scene_.begin() + index);
    }
    // All useless elements are deleted so clear the "deleted array"
    deleted_.clear();

    // If flag is set than it's time to spawn small ones
    // And if we hit meteor at (0, 0), well.. than it's a lucky shot
    if (smallMeteorX_ || smallMeteorY_) {
        for (int i = 0; i < smallMeteors; ++i) {
            SmallMeteor* smallMeteor = new SmallMeteor(smallMeteorX_, smallMeteorY_);
            scene_.push_back(smallMeteor);
        }
        // Clear the spawn flag
        smallMeteorX_ = smallMeteorY_ = 0.0f;
    }
}

void Game::updateMeteor(double dt, vector<Node*>::iterator nodeIt) {
    Meteor* meteor = (Meteor*) (*nodeIt);
    // Move meteor
    meteor->translate(meteor->getXFallSpeed(), dt * meteor->getYFallSpeed());
    // Make it spin
    meteor->rotate(meteor->getRotateSpeed());

    // Mark it for deletion if it's out
    if (meteor->isOut()) {
        deleted_.push_back(nodeIt - scene_.begin());
    }

    // If meteor hit shuttle than the game is over
    if (shuttle_->isIntersect(meteor)) {
        isOver_ = true;
    }
}

void Game::updateBullet(double dt, vector<Node*>::iterator nodeIt) {
    Bullet* bullet = (Bullet*) (*nodeIt);
    // Move the bullet up
    bullet->translate(0.0f, dt * bullet->getSpeed());

    // Mark it for deletion if it's out
    if (bullet->isOut()) {
        deleted_.push_back(nodeIt - scene_.begin());
    }

    // Detect if bullet hit meteor
    for (vector<Node*>::iterator node = scene_.begin(); node < scene_.end(); ++node) {
        enum NodeType type = (*node)->getType();

        // And if it hit...
        if ((type == METEOR || type == SMALL_METEOR) && bullet->isIntersect(*node)) {
            // Remove the bullet
            deleted_.push_back(nodeIt - scene_.begin());
            // Remove the meteor
            deleted_.push_back(node - scene_.begin());

            // And if it is a big one set flag to spawn small meteors
            if (type == METEOR) {
                Meteor* meteor = (Meteor*) (*node);
                smallMeteorX_ = meteor->getX();
                smallMeteorY_ = meteor->getY();

                score_++;
            } else {
                score_ += 2;
            }
        }
    }
}

string Game::getGameOverText() {
    stringstream ss;
    ss << "GAME OVER" << endl << "Your score is " << score_;

    return ss.str();
 }

Game::~Game() {
    for (vector<Node*>::iterator node = scene_.begin(); node < scene_.end(); ++node) {
        delete (*node);
    }
}

#endif