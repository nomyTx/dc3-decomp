#include "synth/Mic.h"
#include "os/Debug.h"
#include "obj/Data.h"
#include "utl/MemMgr.h"

void Mic::Set(const DataArray *data) {
    MILO_ASSERT(data, 0x12);
    SetGain(data->FindFloat("gain"));
    SetDMA(data->FindInt("dma"));
    DataArray *compressorArr = data->FindArray("compressor");
    SetCompressor(compressorArr->Int(1));
    SetCompressorParam(compressorArr->Float(2));
}

RingBuffer::~RingBuffer() {
    if (mBuffer) {
        MemFree(mBuffer, __FILE__, 0x23);
        mBuffer = nullptr;
    }
}

void RingBuffer::Reset() {
    memset(mBuffer, 0, mSize);
    unkc = 0;
    unk10 = 0;
    unk4 = 0;
}

void RingBuffer::Init(int size) {
    mSize = size;
    if (mBuffer) {
        MemFree(mBuffer, __FILE__, 0x2B);
        mBuffer = nullptr;
    }
    mBuffer = MemAlloc(size, __FILE__, 0x2C, "VirtualMic RingBuffer", 0x80);
    MILO_ASSERT(mBuffer, 0x2D);
    Reset();
}
