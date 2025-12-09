#pragma once
#include "win_types.h"

typedef struct __declspec(intrin_type) __declspec(align(16)) __vector4 { /* Size=0x10 */
    union {
        /* 0x0000 */ float vector4_f32[4];
        /* 0x0000 */ UINT vector4_u32[4];
        struct {
            /* 0x0000 */ float x;
            /* 0x0004 */ float y;
            /* 0x0008 */ float z;
            /* 0x000c */ float w;
        };
        /* 0x0000 */ float v[4];
        /* 0x0000 */ UINT u[4];
    };
} __vector4;
typedef __vector4 XMVECTOR;

struct XMVECTORF32 { /* Size=0x10 */
    union {
        /* 0x0000 */ float f[4];
        /* 0x0000 */ __vector4 v;
    };
};

struct XMVECTORI32 { /* Size=0x10 */
    union {
        /* 0x0000 */ int i[4];
        /* 0x0000 */ __vector4 v;
    };
};

struct XMVECTORU32 { /* Size=0x10 */
    union {
        /* 0x0000 */ unsigned int u[4];
        /* 0x0000 */ __vector4 v;
    };
};

typedef struct _XMMATRIX { /* Size=0x40 */
    union {
        /* 0x0000 */ XMVECTOR r[4];
        struct {
            /* 0x0000 */ float _11;
            /* 0x0004 */ float _12;
            /* 0x0008 */ float _13;
            /* 0x000c */ float _14;
            /* 0x0010 */ float _21;
            /* 0x0014 */ float _22;
            /* 0x0018 */ float _23;
            /* 0x001c */ float _24;
            /* 0x0020 */ float _31;
            /* 0x0024 */ float _32;
            /* 0x0028 */ float _33;
            /* 0x002c */ float _34;
            /* 0x0030 */ float _41;
            /* 0x0034 */ float _42;
            /* 0x0038 */ float _43;
            /* 0x003c */ float _44;
        };
        /* 0x0000 */ float m[4][4];
    };

    _XMMATRIX(const float *);
    _XMMATRIX(
        float,
        float,
        float,
        float,
        float,
        float,
        float,
        float,
        float,
        float,
        float,
        float,
        float,
        float,
        float,
        float
    );
    _XMMATRIX(const XMVECTOR, const XMVECTOR, const XMVECTOR, const XMVECTOR);
    _XMMATRIX();
    float &operator()(UINT, UINT);
    float operator()(UINT, UINT) const;
    _XMMATRIX &operator=(const _XMMATRIX &);
    _XMMATRIX &operator*=(const _XMMATRIX &);
    _XMMATRIX operator*(const _XMMATRIX &) const;
} XMMATRIX;

// VMX128 Intrinsics let's goooooooo

static inline XMVECTOR __lvx(const void *base, int offset) {
    const char *ret = (const char *)base + offset;
    return *(XMVECTOR *)ret;
}

static inline void __stvx(XMVECTOR vSrc, void *base, int offset) {
    char *dest = (char *)base + offset;
    XMVECTOR *vDst = (XMVECTOR *)dest;
    *vDst = vSrc;
}

// TODO: need intrinsics for:
// vspltw128, vmaddcfp128, vmaddfp128
// any others that we find in DC3
