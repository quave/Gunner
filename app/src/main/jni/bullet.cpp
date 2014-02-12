#ifndef BULLET_CPP
#define BULLET_CPP

#include <GLES2/gl2.h>
#include <vecmath.h>
#include "node.cpp"

using namespace ndk_helper;

class Bullet: public Node {

protected:

public:
    ~Bullet();
    void init(int width, int height);
    NodeType getType() { return BULLET; };
};

void Bullet::init(int width, int height) {
    LOGI("Init bullet");
    vertexCount_ = 4;

    vertices_ = new GLfloat[vertexCount_ * DIMENTIONS];
    vertices_[0] = 0.0f;    vertices_[1] = 1.0f;
    vertices_[2] = -0.4f;   vertices_[3] = 0.0f;
    vertices_[4] = 0.0f;    vertices_[5] = -1.0f;
    vertices_[6] = 0.4f;    vertices_[7] = 0.0f;

    colors_ = new GLfloat[vertexCount_ * COLOR_COMPONENTS];

    for(int i = 0; i < vertexCount_ * COLOR_COMPONENTS; i+=COLOR_COMPONENTS ) {
        colors_[i] = 0.2078f;
        colors_[i + 1] = 1.0f;
        colors_[i + 2] = 1.0f;
        colors_[i + 3] = 1.0;
    }

    scale(0.08f, 0.08f * width / height);

    translate(0.0f, -0.6f);

    Node::init(width, height);
}

Bullet::~Bullet() {
    if (vertices_ != NULL) { delete [] vertices_; }
    if (colors_ != NULL) { delete [] colors_; }
}

#endif