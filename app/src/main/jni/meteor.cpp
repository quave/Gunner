#ifndef METEOR_CPP
#define METEOR_CPP

#include <GLES2/gl2.h>
#include <math.h>

#include "node.cpp"

class Meteor: public Node {

    void generate();
    float xFallSpeed_;

public:
    Meteor(int w, int h);
    NodeType getType() { return METEOR; };
    bool isOut();
    float getXFallSpeed() { return xFallSpeed_; }
};

Meteor::Meteor(int width, int height)
    : xFallSpeed_(0.0f)
{
    vertexCount_ = rand() % 6 + 4;

    vertices_ = new GLfloat[vertexCount_ * DIMENTIONS];
    generate();

    colors_ = new GLfloat[vertexCount_ * COLOR_COMPONENTS];
    for(int i = 0; i < vertexCount_ * COLOR_COMPONENTS; i+=COLOR_COMPONENTS ) {
        colors_[i] = 0.9608f;
        colors_[i + 1] = 0.3608f;
        colors_[i + 2] = 0.8902f;
        colors_[i + 3] = 1.0;
    }

    scale(0.4f, 0.4f * width / height);
    float x = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
    translate(x, 1.0f);

    xFallSpeed_ = -1.0f * copysignf(1.0, x_) * ((float)rand() / RAND_MAX) * 0.01f;
}

void Meteor::generate() {
    if (vertices_ == NULL || vertexCount_ < 4) {
        return;
    }

    // Generate first point
    float r1 = (float)rand() / RAND_MAX / 2 + 0.5f;
    float a1 = 0;
    float x1 = vertices_[0] = r1;
    float y1 = vertices_[1] = 0;
    float x01 = x1;
    float y01 = y1;

    // Generate second point
    float r2 = (float)rand() / RAND_MAX / 2 + 0.5f;
    float a2 = (2 * M_PI) / vertexCount_;
    float x2 = vertices_[2] = r2 * cos(a2);
    float y2 = vertices_[3] = r2 * sin(a2);
    float x02 = x2;
    float y02 = y2;

    for(int i = 2; i < vertexCount_ - 1; ++i) {
        float a0 = i * (2 * M_PI) / vertexCount_;
        float x0 = cos(a0);
        float y0 = sin(a0);

        float rmax = (y1*x2 - x1*y2) / (y0*(x2-x1) + x0*(y1-y2));
        rmax = fmin(1.0f, rmax);
        if (rmax < 0.0f) { rmax = 1.0f; }
        float rmin = rmax / 2;

        float r = (float)rand() / RAND_MAX * (rmax - rmin) + rmin;
        float x = vertices_[i * 2] = r * x0;
        float y = vertices_[i * 2 + 1] = r * y0;

        x1 = x2;
        y1 = y2;
        x2 = x;
        y2 = y;
    }

    float a0 = (vertexCount_ - 1) * (2 * M_PI) / vertexCount_;
    float x0 = cos(a0);
    float y0 = sin(a0);
    float rmax1 = (y1*x2 - x1*y2) / (y0*(x2-x1) + x0*(y1-y2));
    float rmax2 = (y01*x02 - x01*y02) / (y0*(x02-x01) + x0*(y01-y02));
    float rmax = fmin(rmax1, rmax2);
    rmax = fmin(1.0f, rmax);
    float rmin = (y1*x01 - x1*y01) / (y0*(x01-x1) + x0*(y1-y01));
    if (rmax < 0.0f) { rmax = 1.0f; rmin=0.5f; }

    float r = (float)rand() / RAND_MAX * (rmax - rmin) + rmin;

    int index = (vertexCount_ - 1) * 2;
    vertices_[index] = r * x0;
    vertices_[index + 1] = r * y0;
}

bool Meteor::isOut() {
    if (vertices_ == NULL) { return false; }

    float xmin = XMAX;
    float xmax = XMIN;
    float ymax = YMIN;

    for (int i = 0; i < vertexCount_; ++i) {
        float x = vertices_[i * 2];
        float y = vertices_[i * 2 + 1];

        if (x < xmin) { xmin = x; }
        if (x > xmax) { xmax = x; }
        if (y > ymax) { ymax = y; }
    }

    return  xmax <= XMIN || xmin >= XMAX || ymax <= YMIN;
}

#endif