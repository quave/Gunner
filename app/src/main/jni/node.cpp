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
    float angle_;
    virtual void scale(float, float);

public:
    Node():
        vertices_(NULL),
        colors_(NULL),
        vertexCount_(0),
        x_(0.0f), y_(0.0f),
        angle_(0.0f) {};
    ~Node();
    virtual void translate(float, float);
    virtual void rotate(float);
    virtual void draw(double dt, GLuint hPos, GLuint hCol, GLuint hVP, Mat4 mVP);
    int getVertexCount() {return vertexCount_;};
    virtual NodeType getType() { return NODE; };
    virtual bool isOut();
    float getX() { return x_; };
    float getY() { return y_; };

    bool isInside(float, float);
};

void Node::scale(float sx, float sy) {
    if (vertices_ == NULL) { return; }

    for (int i = 0; i < vertexCount_ * 2; i+=2 ) {
        vertices_[i] *= sx;
        vertices_[i+1] *= sy;
    }
}

void Node::translate(float tx, float ty) {
    x_ += tx;
    y_ += ty;
}

void Node::rotate(float angle) {
    angle_ += angle;
}

void Node::draw(double dt, GLuint hPos, GLuint hCol, GLuint hVP, Mat4 mVP)
{
    if (vertices_ == NULL) { return; }

    Mat4 rot = Mat4::RotationZ(angle_);
    Mat4 tran = Mat4::Translation(x_, y_, 0.0f);
    Mat4 transform = mVP * tran * rot;

    glVertexAttribPointer(hPos, 2, GL_FLOAT, GL_FALSE, 0, vertices_);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(hPos);
    checkGlError("glEnableVertexAttribArray");

    glVertexAttribPointer(hCol, 4, GL_FLOAT, GL_FALSE, 0, colors_);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(hCol);
    checkGlError("glEnableVertexAttribArray");

    glUniformMatrix4fv(hVP, 1, GL_FALSE, transform.Ptr());
    checkGlError("glUniformMatrix4fv");

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
        float curX = vertices_[i * 2] + x_, curY = vertices_[i * 2 + 1] + y_;
        float prevX = vertices_[j * 2] + x_, prevY = vertices_[j * 2 + 1] + y_;

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