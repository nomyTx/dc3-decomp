#include "hamobj/FreestyleMoveRecorder.h"
#include "obj/Object.h"
#include "rndobj/Tex.h"

FreestyleMoveRecorder::FreestyleMoveRecorder()
    : unk4(0), unk8(0), unkc(0), unk18(0), unk20(-1), unk24(60), unk28(-1), unk2c(-1),
      unk30(15), unk34(-1), unk38(0), unk39(0), unk44(-1), unkb8(0) {
    unkbc = Hmx::Object::New<RndTex>();
    unkbc->SetBitmap(320, 240, 16, RndTex::kTexRegularLinear, false, nullptr);
}
