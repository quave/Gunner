#ifndef SMALL_METEOR_CPP
#define SMALL_METEOR_CPP

#include <GLES2/gl2.h>
#include <math.h>

#include "node.cpp"

class SmallMeteor: public Meteor {

public:
    SmallMeteor(int w, int h, float x, float y);
    NodeType getType() { return SMALL_METEOR; };
};

SmallMeteor::SmallMeteor(int w, int h, float x, float y)
    : Meteor(w, h) {
    //LOGI("Init small meteor");

    scale(0.3f, 0.3f);
    translate(x - x_, y - y_);

    //LOGI("Init end small meteor (%1.4f, %1.4f)", x_, y_);
}

#endif