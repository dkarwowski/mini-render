#ifndef _GEOMETRY_h_
#include "math.h"

#define DECLARE_V2(type) \
    union v2_##type { \
        struct { type x, y; }; \
        struct { type u, v; }; \
        type raw[2]; \
    }

#define DEFINE_V2(type) \
    inline union v2_##type V2_##type(type a, type b) { \
        union v2_##type result = {.x=a, .y=b}; \
        return result; \
    } \
    inline union v2_##type AddV2_##type(union v2_##type a, union v2_##type b) {\
        union v2_##type result = {.x=a.u + b.u, .y=a.v + b.v}; \
        return result; \
    } \
    inline union v2_##type SubV2_##type(union v2_##type a, union v2_##type b) {\
        union v2_##type result = {.x=a.u - b.u, .y=a.v - b.v}; \
        return result; \
    } \
    inline union v2_##type MulV2_##type(float f, union v2_##type a) { \
        union v2_##type result = {.x=f * a.u, .y=f * a.v}; \
        return result; \
    }

#define DECLARE_V3(type) \
    union v3_##type { \
        struct { type x, y, z; }; \
        struct { type ivert, iuv, inorm; }; \
        type raw[3]; \
    }

#define DEFINE_V3(type) \
    inline union v3_##type V3_##type(type a, type b, type c) { \
        union v3_##type result = {.x=a, .y=b, .z=c}; \
        return result; \
    } \
    inline union v3_##type AddV3_##type(union v3_##type a, union v3_##type b) { \
        union v3_##type result = {.x=a.x + b.x, .y=a.y + b.y, .z=a.z + b.z}; \
        return result; \
    } \
    inline union v3_##type SubV3_##type(union v3_##type a, union v3_##type b) { \
        union v3_##type result = {.x=a.x - b.x, .y=a.y - b.y, .z=a.z - b.z}; \
        return result; \
    } \
    inline union v3_##type MulV3_##type(float f, union v3_##type a) { \
        union v3_##type result = {.x=f * a.x, .y=f * a.y, .z=f * a.z}; \
        return result; \
    } \
    inline union v3_##type NormV3_##type(union v3_##type a) { \
        union v3_##type result = MulV3_##type(1.0f/sqrt(a.x*a.x+a.y*a.y+a.z*a.z), a); \
        return result; \
    }

typedef DECLARE_V2(int) v2i;
typedef DECLARE_V2(float) v2f;
typedef DECLARE_V3(int) v3i;
typedef DECLARE_V3(float) v3f;

DEFINE_V2(int);
DEFINE_V2(float);
DEFINE_V3(int);
DEFINE_V3(float);

#define _GEOMETRY_h_
#endif
