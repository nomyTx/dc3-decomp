#include "rnddx9/CubeTex.h"
#include "rnddx9/Rnd.h"
#include "rndobj/Mat_NG.h"

DxCubeTex::DxCubeTex() : mResource(0) {}
DxCubeTex::~DxCubeTex() { Reset(); }

void DxCubeTex::Select(int x) {
    // D3DDevice_SetTexture
}

void DxCubeTex::Reset() {
    TheDxRnd.AutoRelease(mResource);
    mResource = nullptr;
    NgMat::SetCurrent(nullptr);
}
