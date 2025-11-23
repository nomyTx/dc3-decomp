#pragma once
#include "gesture/BaseSkeleton.h"
#include "hamobj/DancerSkeleton.h"
#include "utl/MemMgr.h"

struct FreestyleMoveFrame {
    DancerSkeleton skeleton; // 0x0

    static void *operator new[](unsigned int s) {
        return _MemAllocTemp(s, __FILE__, 0x10, "FreestyleMoveFrame", 0);
    }
    static void operator delete[](void *v) {
        MemFree(v, __FILE__, 0x10, "FreestyleMoveFrame");
    }
};

struct DepthFrame {
    static void *operator new(unsigned int s) {
        return _MemAllocTemp(s, __FILE__, 0x26, "DepthFrame", 0);
    }
    static void operator delete(void *v) { MemFree(v, __FILE__, 0x26, "DepthFrame"); }
};

// size 0x1c
class FreestyleMove {
public:
    FreestyleMove();
    virtual ~FreestyleMove();

    void Clear();
    void Init(int);
    void CalcCentering(int);
    void Free();
    void RecordSkeletonFrame(BaseSkeleton *, int, float);

    MEM_OVERLOAD(FreestyleMove, 0x18);

private:
    DepthFrame *unk4; // 0x4
    int unk8;
    int unkc;
    int unk10;
    int unk14;
    FreestyleMoveFrame *unk18; // 0x18
};

// size 0x10
class FreeStyleFrameScores {
public:
private:
    std::vector<float> unk0;
    int unkc;
};
