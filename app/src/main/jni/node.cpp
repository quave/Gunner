#ifndef NODE_CPP
#define NODE_CPP

#include <GLES2/gl2.h>
#include <vecmath.h>

#define DIMENTIONS 2
#define COLOR_COMPONENTS 4
#define XMIN -1.0f
#define XMAX 1.0
#define YMIN -1.0
#define YMAX 1.0f

using namespace ndk_helper;

enum NodeType {
    NODE,
    SHUTTLE,
    BULLET,
    METEOR,
    SMALL_METEOR
};

class Node {

protected:
    GLfloat* vertices_;
    GLfloat* colors_;
    int vertexCount_;
    float x_;
    float y_;

public:
    Node():
        vertices_(NULL),
        colors_(NULL),
        vertexCount_(0),
        x_(0.0f), y_(0.0f) {};
    ~Node();
    virtual void scale(float, float);
    virtual void translate(float, float);
    virtual void rotate(float);
    virtual void draw(double dt, GLuint hPos, GLuint hCol);
    int getVertexCount() {return vertexCount_;};
    virtual NodeType getType() { return NODE; };
    virtual bool isOut();
    float getX() { return x_; };
    float getY() { return y_; };

    bool isInside(float x, float y);
};

void Node::scale(float sx, float sy) {
    if (vertices_ == NULL) { return; }

    float x = x_;
    float y = y_;

    if (x && y) { translate(-x_, -y_); }

    for (int i = 0; i < vertexCount_ * 2; i+=2 ) {
        vertices_[i] *= sx;
        vertices_[i+1] *= sy;
    }

    if (x && y) { translate(x, y); }
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

    x_ += tx;
    y_ += ty;
}

void Node::rotate(float angle) {
    if (vertices_ == NULL) { return; }

    float x = x_;
    float y = y_;

    if (x && y) { translate(-x_, -y_); }

    Mat4 rot = Mat4::RotationZ(angle);

    for (int i = 0; i < vertexCount_ * 2; i+=2 ) {
        Vec4 vec(vertices_[i], vertices_[i+1], 0.0f, 1.0f);
        vec = rot * vec;
        float x, y, dumm;
        vec.Value(x, y, dumm, dumm);
        vertices_[i] = x;
        vertices_[i+1] = y;
    }

    if (x && y) { translate(x, y); }
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

bool Node::isOut() {
    if (vertices_ == NULL) { return false; }

    float xmin = XMAX;
    float xmax = XMIN;
    float ymin = YMAX;
    float ymax = YMIN;

    for (int i = 0; i < vertexCount_; ++i) {
        float x = vertices_[i * 2];
        float y = vertices_[i * 2 + 1];

        if (x < xmin) { xmin = x; }
        if (x > xmax) { xmax = x; }
        if (y < ymin) { ymin = y; }
        if (y > ymax) { ymax = y; }
    }

    return  xmax <= XMIN || xmin >= XMAX || ymax <= YMIN || ymin >= YMAX;
}

bool Node::isInside(float x, float y) {
    if (vertices_ == NULL) { return false; }

    bool result = false;

    for (int i = 0, j = vertexCount_ - 1; i < vertexCount_; j = i++) {
        float curX = vertices_[i * 2], curY = vertices_[i * 2 + 1];
        float prevX = vertices_[j * 2], prevY = vertices_[j * 2 + 1];

        if ((curY > y) != (prevY > y) &&
            (x < (prevX - curX) * (y - curY) / (prevY - curY) + curX)) {
            result = !result;
        }
    }

    return result;
}

Node::~Node() {
    if (vertices_ != NULL) { delete [] vertices_; }
    if (colors_ != NULL) { delete [] colors_; }
}

#endif