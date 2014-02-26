#ifndef BULLET_CPP
#define BULLET_CPP

#include <GLES2/gl2.h>
#include "node.cpp"

class Bullet: public Node {

protected:

public:
    Bullet(int w, int h);
    NodeType getType() { return BULLET; };
    bool isIntersect(Node* node);
};

Bullet::Bullet(int width, int height) {
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
}

bool Bullet::isIntersect(Node* node) {
    if (vertices_ == NULL) {
        return false;
    }

    float x = vertices_[0], y = vertices_[1];
    return node->isInside(x, y);
}

#endif