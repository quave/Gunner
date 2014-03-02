#ifndef SMALL_METEOR_CPP
#define SMALL_METEOR_CPP

#include <GLES2/gl2.h>
#include <math.h>

#include "meteor.cpp"

class SmallMeteor: public Meteor {

public:
    SmallMeteor(float x, float y);
    NodeType getType() { return SMALL_METEOR; };
};

SmallMeteor::SmallMeteor(float x, float y)
    : Meteor() {
    // Make it small
    scale(0.3f, 0.3f);
    // Translate from current coordinates to the specified
    translate(x - x_, y - y_);
    // Update x speed
    updateXSpeed();
}

#endif