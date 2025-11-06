#include "utl/DeJitter.h"
#include "obj/Data.h"

float DeJitter::sTimeScale = 1;

DeJitter::DeJitter() { Reset(); }

void DeJitter::Reset() {
    unk80 = 0;
    unk84 = -2;
    unk88 = 0;
    unk8c = 0;
    for (int i = 0; i < 32; i++) {
        unk0[i] = 0;
    }
}

float DeJitter::NewMs(float f1, float &fref) {
    static DataNode &n = DataVariable("dejitter_disable");
    if (!n.Int() && unk84 > 8) {
        int unk80diff = unk80 - 1;
        float fdiff = (unk0[unk80diff] - unk0[unk80diff - unk84]) / unk84;
        if (unk88 == 0) {
            unk88 = fdiff;
        }
        unk88 = (fdiff - unk88) * 0.1f + unk88;
        if (sTimeScale == 1) {
        } else {
        }
    }
    return unk8c;
}
