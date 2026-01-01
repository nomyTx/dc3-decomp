#include "hamobj/FreestyleMove.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/DancerSkeleton.h"

FreestyleMove::FreestyleMove() : unk4(0), mNumFrames(0), unk10(0), unk14(0), unk18(0) {}

FreestyleMove::~FreestyleMove() {
    delete[] unk4;
    delete[] unk18;
}

void FreestyleMove::Clear() { mNumFrames = 0; }

void FreestyleMove::Free() {
    mNumFrames = 0;
    delete[] unk4;
    delete[] unk18;
    unk4 = nullptr;
    unk18 = nullptr;
}

void FreestyleMove::Init(int i1) {
    mNumFrames = 0;
    if (!unk4) {
        unk4 = new DepthFrame[i1];
    }
    if (!unk18) {
        unk18 = new FreestyleMoveFrame[i1];
    }
}

void FreestyleMove::RecordSkeletonFrame(BaseSkeleton *skeleton, int i2, float f3) {
    FreestyleMoveFrame frame;
    frame.skeleton.Init();
    frame.unk2d8 = f3;
    if (skeleton && skeleton->IsTracked()) {
        frame.skeleton.Set(*skeleton);
    }
    unk18[i2] = frame;
}
