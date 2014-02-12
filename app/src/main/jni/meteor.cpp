#ifndef METEOR_CPP
#define METEOR_CPP

#include <GLES2/gl2.h>
#include <math.h>
#include <vecmath.h>
#include "node.cpp"

using namespace ndk_helper;

class Meteor: public Node {

public:
    ~Meteor();
    void init(int width, int height);
    NodeType getType() { return METEOR; };
};

void Meteor::init(int width, int height) {
    LOGI("Init meteor");

    vertexCount_ = rand() % 6 + 4;
    LOGI ("Meteor vertex count = %d", vertexCount_);

    vertices_ = new GLfloat[vertexCount_ * DIMENTIONS];
    for(int i = 0; i < vertexCount_ * DIMENTIONS; i+=DIMENTIONS ) {
        float r = (float)rand() / RAND_MAX;
        float a = (i / DIMENTIONS) * (2 * M_PI) / vertexCount_;
        vertices_[i] = r * cos(a);
        vertices_[i + 1] = r * sin(a);
    }

    colors_ = new GLfloat[vertexCount_ * COLOR_COMPONENTS];
    for(int i = 0; i < vertexCount_ * COLOR_COMPONENTS; i+=COLOR_COMPONENTS ) {
        colors_[i] = 0.9608f;
        colors_[i + 1] = 0.3608f;
        colors_[i + 2] = 0.8902f;
        colors_[i + 3] = 1.0;
    }

    scale(0.4f, 0.4f * width / height);
    translate(((float)rand() / RAND_MAX) - 1.0f, 1.0f);

    Node::init(width, height);
}

Meteor::~Meteor() {
    if (vertices_ != NULL) { delete [] vertices_; }
    if (colors_ != NULL) { delete [] colors_; }
}



#endif