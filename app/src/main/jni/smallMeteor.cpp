#ifndef SMALL_METEOR_CPP
#define SMALL_METEOR_CPP

#include <GLES2/gl2.h>
#include <math.h>

#include "meteor.cpp"

class SmallMeteor: public Meteor {

public:
    SmallMeteor(int w, int h, float x, float y);
    NodeType getType() { return SMALL_METEOR; };
};

SmallMeteor::SmallMeteor(int w, int h, float x, float y)
    : Meteor(w, h) {
    // Make it small
    scale(0.3f, 0.3f);
    // Translate from current coordinates to the specified
    translate(x - x_, y - y_);
}

#endif