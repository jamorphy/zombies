#include "math_utils.h"

void vec3_copy(vec3 dst, const vec3 src) {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
}

void quat_copy(quat dst, const quat src) {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
}
