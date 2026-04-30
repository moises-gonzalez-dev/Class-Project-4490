#ifndef CAMERA_H
#define CAMERA_H

#include "math_utils.h"

class Camera {
public:
    Vec position;
    Vec last_position;
    Vec direction;
    Vec force;
    Vec vel;

    Camera() {
    VecMake(0, 3, 0, position);   // start inside room, Y=3 is ground level
    VecMake(0, 0, -1, direction);
    VecMake(0, 0, 0, force);
    VecMake(0, 0, 0, vel);
    VecMake(0, 0, 0, last_position);
}


    void translate(float x, float y, float z) {
        position[0] += x;
        position[1] += y;
        position[2] += z;
    }

    void lookLeftRight(float angle) {
        Matrix mat;
        identity33(mat);
        Vec rot = {0.0, angle, 0.0};
        yy_transform(rot, mat);
        trans_vector(mat, direction, direction);
    }

    void lookUpDown(float angle) {
        Vec left;
        Vec up = {0, 1, 0};
        VecCross(direction, up, left);
        Matrix mat;
        identity33(mat);
        matrixFromAxisAngle(left, angle, mat);
        trans_vector(mat, direction, direction);
    }

    void moveLeftRight(float dist) {
        Vec left;
        Vec up = {0, 1, 0};
        VecCross(direction, up, left);
        position[0] += left[0] * dist;
        position[1] += left[1] * dist;
        position[2] += left[2] * dist;
    }

    void moveUpDown(float dist) {
        position[1] += dist;
    }
};

#endif // CAMERA_H
