#ifndef NODE_CPP
#define NODE_CPP

#include <GLES2/gl2.h>
#include <vecmath.h>

#define DIMENTIONS 2
#define COLOR_COMPONENTS 4

using namespace ndk_helper;

enum NodeType{
    NODE,
    SHUTTLE,
    BULLET,
    METEOR
};

class Node {

protected:
    GLfloat* vertices_;
    GLfloat* colors_;
    int vertexCount_;

public:
    Node();
    virtual void init(int width, int height) { LOGI("Node init"); };
    virtual void scale(float, float);
    virtual void translate(float, float);
    virtual void rotate(float);
    virtual void draw(double dt, GLuint hPos, GLuint hCol);
    int getVertexCount() {return vertexCount_;};
    virtual NodeType getType() { return NODE; };
};

Node::Node() {
    vertices_ = NULL;
    colors_ = NULL;
    vertexCount_ = 0;
}

void Node::scale(float sx, float sy) {
    if (vertices_ == NULL) { return; }

    for (int i = 0; i < vertexCount_ * 2; i+=2 ) {
        vertices_[i] *= sx;
        vertices_[i+1] *= sy;
    }
}

void Node::translate(float tx, float ty) {
    if (vertices_ == NULL) { return; }

    Mat4 trans = Mat4::Translation(tx, ty, 0.0f);

    for (int i = 0; i < vertexCount_ * 2; i+=2 ) {
        Vec4 vec(vertices_[i], vertices_[i+1], 0.0f, 1.0f);
        vec = trans * vec;
        float x, y, dumm;
        vec.Value(x, y, dumm, dumm);
        vertices_[i] = x;
        vertices_[i+1] = y;
    }
}

void Node::rotate(float angle) {
    if (vertices_ == NULL) { return; }

    Mat4 rot = Mat4::RotationZ(angle);

    for (int i = 0; i < vertexCount_ * 2; i+=2 ) {
        Vec4 vec(vertices_[i], vertices_[i+1], 0.0f, 1.0f);
        vec = rot * vec;
        float x, y, dumm;
        vec.Value(x, y, dumm, dumm);
        vertices_[i] = x;
        vertices_[i+1] = y;
    }
}

void Node::draw(double dt, GLuint hPos, GLuint hCol)
{
    if (vertices_ == NULL) { return; }

    glVertexAttribPointer(hPos, 2, GL_FLOAT, GL_FALSE, 0, vertices_);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(hPos);
    checkGlError("glEnableVertexAttribArray");

    glVertexAttribPointer(hCol, 4, GL_FLOAT, GL_FALSE, 0, colors_);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(hCol);
    checkGlError("glEnableVertexAttribArray");

    glDrawArrays(GL_LINE_LOOP, 0, vertexCount_);
    checkGlError("glDrawArrays");
}

#endif