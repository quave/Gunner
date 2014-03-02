#ifndef SHUTTLE_CPP
#define SHUTTLE_CPP

#include <GLES2/gl2.h>
#include "node.cpp"

class Shuttle: public Node {

    static const float speed = 0.15f;

public:
    Shuttle();
    NodeType getType() { return SHUTTLE; };
    bool isIntersect(Node* node);
    float getSpeed() { return speed; }
};

Shuttle::Shuttle() {
    vertexCount_ = 3;

    vertices_ = new GLfloat[vertexCount_ * DIMENTIONS];
    vertices_[0] = 0.0f;    vertices_[1] = 1.0f;
    vertices_[2] = -0.5f;   vertices_[3] = 0.0f;
    vertices_[4] = 0.5f;    vertices_[5] = 0.0f;

    colors_ = new GLfloat[vertexCount_ * COLOR_COMPONENTS];
    for(int i = 0; i < vertexCount_ * COLOR_COMPONENTS; i+=COLOR_COMPONENTS ) {
        colors_[i] = 0.3686f;
        colors_[i + 1] = 1.0f;
        colors_[i + 2] = 0.1529f;
        colors_[i + 3] = 1.0;
    }

    scale(0.15f, 0.15f);
    y_ = -0.95f;
}

bool Shuttle::isIntersect(Node* node) {
    if (vertices_ == NULL) {
        return false;
    }

    for (int i = 0; i < vertexCount_; ++i) {
        float x = vertices_[i * 2] + x_;
        float y = vertices_[i * 2 + 1] + y_;

        if (node->isInside(x, y)) {
            return true;
        }
    }

    return false;
}

#endif