#pragma once
#include "os/Debug.h"
#include "rndobj/Rnd_NG.h"
#include "xdk/D3D9.h"
#include <types.h>

class DxRnd : public NgRnd {
public:
    int unk220;
    D3DDevice *mD3DDevice; // 0x224
    u8 pad2[0xD9];
    u8 unk_0x301;

    DxRnd(void);

    D3DDevice *D3DDevice() { return mD3DDevice; }
    void AutoRelease(class D3DResource *);

    static const char *Error(s32);
};

#define MILO_PRINT_D3DERR(err_result, line)                                              \
    TheDebug.Fail(                                                                       \
        MakeString(                                                                      \
            "File: %s Line: %d Error: %s\n", __FILE__, line, DxRnd::Error(err_result)    \
        ),                                                                               \
        0                                                                                \
    )

extern DxRnd TheDxRnd;
