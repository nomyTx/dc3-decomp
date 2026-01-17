#include "gesture/LiveCameraInput.h"
#include "DrawUtl.h"
#include "gesture/CameraInput.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonUpdate.h"
#include "gesture/SpeechMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rnddx9/Tex.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Tex.h"
#include "utl/MemTrack.h"
#include "utl/Std.h"
#include "xdk/nui/nuiapi.h"
#include "xdk/nui/nuiaudio.h"
#include "xdk/nui/nuidetroit.h"
#include "xdk/win_types.h"

namespace {
    // bool  GetExposureRegion(struct _NUI_CAMERA_AE_ROI &)
    // long  GetColorCameraProperty(  _NUI_CAMERA_PROPERTY)

    void SetColorCameraProperty(NUI_CAMERA_PROPERTY, long);

    // void  LoadDebugDepthBuffer(class RndTex *&)
}

LiveCameraInput *LiveCameraInput::sInstance;
int g_ColorPollCnt;
int g_ColorNoFrameDataCnt;

#pragma region TextureStore

void LiveCameraInput::TextureStore::StoreTexture(RndTex *tex) {
    if (mTex) {
        RELEASE(mTex);
    }
    if (tex) {
        mTex = Hmx::Object::New<RndTex>();
        RndBitmap bitmap;
        tex->LockBitmap(bitmap, 1);
        mTex->SetBitmap(bitmap, nullptr, true, RndTex::kTexRenderedNoZ);
        tex->UnlockBitmap();
    } else {
        mTex = nullptr;
    }
}

void LiveCameraInput::TextureStore::StoreColorBuffer(LiveCameraInput *cam) {
    if (mTex) {
        if (mTex->Width() == 640 && mTex->Height() == 480)
            goto update;
        RELEASE(mTex);
    }
    mTex = Hmx::Object::New<RndTex>();
    mTex->SetBitmap(640, 480, 16, RndTex::kScratch, false, nullptr);
    MILO_ASSERT(mTex, 0x53A);
    MILO_ASSERT(mTex->Bpp() == 16, 0x53B);
    MILO_ASSERT(mTex->GetType() == RndTex::kScratch, 0x53C);
update:
    UpdateFromColorBuffer(cam);
}

void LiveCameraInput::TextureStore::StoreDepthBuffer(LiveCameraInput *cam) {
    if (mTex) {
        if (mTex->Width() == 640 && mTex->Height() == 480)
            goto update;
        RELEASE(mTex);
    }
    mTex = Hmx::Object::New<RndTex>();
    mTex->SetBitmap(640, 480, 16, RndTex::kScratch, false, nullptr);
    MILO_ASSERT(mTex, 0x602);
    MILO_ASSERT(mTex->Bpp() == 16, 0x603);
    MILO_ASSERT(mTex->GetType() == RndTex::kScratch, 0x604);
update:
    UpdateFromDepthBuffer(cam);
}

#pragma endregion
#pragma region LiveCameraInput

LiveCameraInput::LiveCameraInput()
    : mConnected(true), unk11e9(0), unk11ea(0), unk11eb(0), unk11ec(0), mSpeechMgr(0) {
    for (int i = 0; i < 8; i++) {
        mTexClips[i].mTex = nullptr;
    }
    mSpeechMgr = nullptr;
    mNumSnapshots = 0;
    unk14a8 = 0;
    unk14ac = 0;
    unk14b0 = 0;
    mSnapshotBatches.clear();
    mNumSnapshots = 0;
    SkeletonUpdate::Init();
    DataArray *kinectArr = SystemConfig()->FindArray("kinect", false);
    bool b17 = false;
    if (kinectArr) {
        DataArray *speechArr = kinectArr->FindArray("speech");
        b17 = speechArr->FindArray("enabled")->Int(1);
    }

    // 0x1454 loop
    //       pLVar15 = this + 0x1454;
    //   lVar18 = 4;
    //   do {
    //     *(undefined4 *)(pLVar15 + -0xc) = 0; 1448
    //     lVar18 = lVar18 + -1;
    //     *(undefined4 *)(pLVar15 + -8) = 0; 144c
    //     *(undefined4 *)(pLVar15 + -4) = 0; 1450
    //     *(undefined4 *)pLVar15 = 0;
    //     *(undefined4 *)(pLVar15 + 4) = 1; 1458
    //     *(undefined4 *)(pLVar15 + 8) = 0; 145c
    //     pLVar15 = pLVar15 + 0x18;
    //   } while (lVar18 != 0);

    int initFlags = 0x4049;
    if (!UsingCD()) {
        initFlags = 0x40004049;
    }
    BeginMemTrackObjectName("NuiInitialize");
    HRESULT initRes = NuiInitialize(initFlags, -1);
    EndMemTrackObjectName();
    if (initRes == E_NUI_DATABASE_NOT_FOUND) {
        MILO_NOTIFY(
            "Could not find NUI database.  Do you have Map DVD Drive enabled in Visual Studio?"
        );
    }
    MILO_ASSERT_FMT(SUCCEEDED(initRes), "NuiInitialize failed (0x%x)", initRes);
    if (b17) {
        mSpeechMgr = new SpeechMgr(kinectArr);
    }
    unk11d4 = 0;
    if (SUCCEEDED(NuiAudioCreate(5, NuiAudioErrorCallback, 1, &unk11d8, nullptr))) {
        NuiAudioRegisterCallbacks(&unk11d8, 1, NuiAudioDataCallback);
        unk11d4 = 1;
    }
    bool i6 = kinectArr->FindArray("title_tracked_skeletons")->Int(1);
    HANDLE new_skeleton_event = SkeletonUpdate::NewSkeletonEvent();
    MILO_ASSERT(new_skeleton_event, 0x14D);
    MILO_ASSERT_FMT(
        SUCCEEDED(NuiSkeletonTrackingEnable(new_skeleton_event, i6 ? 2 : 0)),
        "NuiSkeletonTrackingEnable failed"
    );
    BeginMemTrackObjectName("NuiImageStreamOpen:color");
    MILO_ASSERT_FMT(
        SUCCEEDED(NuiImageStreamOpen(
            NUI_IMAGE_TYPE_COLOR_YUV,
            NUI_IMAGE_RESOLUTION_640x480,
            0,
            2,
            nullptr,
            mStreams[kBufferColor].unk0
        )),
        "NuiImageStreamOpen color failed"
    );
    EndMemTrackObjectName();
    mStreams[kBufferColor].unk14 = CreateCameraBufferMat(640, 480, RndTex::kScratch);
    BeginMemTrackObjectName("NuiImageStreamOpen:depth");
    MILO_ASSERT_FMT(
        SUCCEEDED(NuiImageStreamOpen(
            NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX_IN_COLOR_SPACE,
            NUI_IMAGE_RESOLUTION_320x240,
            0,
            2,
            0,
            mStreams[kBufferDepth].unk0
        )),
        "NuiImageStreamOpen depth failed"
    );
    EndMemTrackObjectName();
    mStreams[kBufferDepth].unk14 = CreateCameraBufferMat(320, 240, RndTex::kScratch);
    mStreams[kBufferPlayer].unk14 = CreateCameraBufferMat(320, 240, RndTex::kScratch);
    mStreams[kBufferPlayerColor].unk14 =
        CreateCameraBufferMat(640, 480, RndTex::kScratch);
    RELEASE(unk14a8);
    unk14a8 = Hmx::Object::New<DxTex>();
    RELEASE(unk14ac);
    unk14ac = Hmx::Object::New<DxTex>();
    DataArray *maxArr = kinectArr->FindArray("camera")->FindArray("max_snapshots", false);
    if (maxArr) {
        mMaxSnapshots = maxArr->Int(1);
    } else {
        MILO_NOTIFY("Could not find max_snapshots in SystemConfig");
        mMaxSnapshots = 1;
    }
    mSnapshotBatches.reserve(6);
    SetColorCameraProperty(NUI_CAMERA_PROPERTY_AE_AWB_MODE, 1);
}

LiveCameraInput::~LiveCameraInput() {}

void LiveCameraInput::PollTracking() {
    unk11e9 = false;
    unk11ea = false;
    unk11eb = false;
    unk11ec = false;
    PollNewStream(kBufferColor);
    CameraInput::PollTracking();
}

void LiveCameraInput::Init() {
    PreInit();
    if (sInstance) {
        DataArray *cfg = SystemConfig()->FindArray("kinect", false);
        if (cfg) {
            DataArray *speechArr = cfg->FindArray("speech");
            cfg = speechArr;
            speechArr->FindInt("enabled");
        }
        if (sInstance->mSpeechMgr) {
            sInstance->mSpeechMgr->InitGrammars(cfg);
        }
    }
}

void LiveCameraInput::Terminate() {
    if (!sInstance) {
        MILO_ASSERT(sInstance, 0xE2);
    }
    RELEASE(sInstance);
}

RndMat *LiveCameraInput::GetSnapshot(int idx) const {
    if (idx >= 0 && idx < mNumSnapshots) {
        return mSnapshots[idx];
    } else if (mNumSnapshots > 0) {
        MILO_LOG("Snapshot index %d out of bounds [0-%d].", idx, mNumSnapshots);
    }
    return nullptr;
}

int LiveCameraInput::GetSnapshotBatchStartingIndex(int idx) const {
    return idx >= mSnapshotBatches.size() ? mNumSnapshots : mSnapshotBatches[idx];
}

RndTex *LiveCameraInput::GetStoredTexture(int idx) const {
    if (idx >= 0 && idx < mTextureStore.size()) {
        return mTextureStore[idx].mTex;
    } else {
        // lol did they forget the other %d
        MILO_LOG(
            "LiveCameraInput::GetStoredTexture: index %d out of bounds [max=%d]\n",
            mTextureStore.size() - 1
        );
        return nullptr;
    }
}

void LiveCameraInput::InitSnapshots(int numSnapshots) {
    ClearSnapshots();
    if (numSnapshots > 0) {
        MILO_ASSERT(numSnapshots <= mMaxSnapshots, 0x193);
        if (mSnapshots.size() != numSnapshots) {
            mSnapshots.resize(numSnapshots);
            for (int i = 0; i < numSnapshots; i++) {
                mSnapshots[i] = CreateCameraBufferMat(640, 480, RndTex::kTexRendered);
            }
        }
    }
}

void LiveCameraInput::InitTextureStore(int max) {
    mMaxTextures = max;
    ClearTextureStore();
}

void LiveCameraInput::ClearTextureStore() {
    for (int i = 0; i < mTextureStore.size(); i++) {
        RELEASE(mTextureStore[i].mTex);
    }
    mTextureStore.clear();
    mTextureStore.resize(mMaxTextures);
    mNumStoredTextures = 0;
}

void LiveCameraInput::StartSnapshotBatch() { mSnapshotBatches.push_back(mNumSnapshots); }

void LiveCameraInput::StoreTextureClipAt(
    float f1, float f2, float f3, float f4, int idx1, int idx2
) {
    if (idx1 >= 0 && idx1 < mTextureStore.size()) {
        mTexClips[idx2].StoreTextureClip(mTextureStore[idx1].mTex, f1, f2, f3, f4);
    } else {
        MILO_LOG(
            "LiveCameraInput::StoreColorBufferClip: index %d out of bounds [max=%d]\n",
            idx1,
            mTextureStore.size() - 1
        );
    }
}

void LiveCameraInput::ResetSnapshots() {
    mNumSnapshots = 0;
    mSnapshotBatches.clear();
}

void LiveCameraInput::SetNewFrame(const SkeletonFrame *frame) {
    mNewFrame = frame;
    mCachedFrame = *frame;
}

int LiveCameraInput::NumSnapshots() const { return mNumSnapshots; }
int LiveCameraInput::NumStoredTextures() const { return mNumStoredTextures; }

void LiveCameraInput::PollNewStream(BufferType buf) {
    MILO_ASSERT(kBufferColor == buf || kBufferDepth == buf, 0x227);
    MILO_ASSERT_RANGE(buf, 0, DIM(mStreams), 0x22C);
    Buffer &curBuf = mStreams[buf];
    if (curBuf.unk0[0]) {
        HRESULT hr = NuiImageStreamGetNextFrame(
            curBuf.unk0[0], 0, (const NUI_IMAGE_FRAME **)&curBuf.unk0[curBuf.unkc + 1]
        );
        if (buf == kBufferColor) {
            g_ColorPollCnt++;
            unk11e9 = true;
        } else {
            unk11ea = true;
        }
        if (SUCCEEDED(hr)) {
            if (buf == kBufferColor) {
                unk11eb = true;
            } else {
                unk11ec = true;
            }
            mConnected = true;
            unk14a8->SetDeviceTex(nullptr);
            unk14ac->SetDeviceTex(nullptr);
            if ((const NUI_IMAGE_FRAME **)curBuf.unk0[curBuf.unk10 + 1]) {
                HRESULT hr = NuiImageStreamReleaseFrame(
                    curBuf.unk0[0], (const NUI_IMAGE_FRAME *)curBuf.unk0[curBuf.unk10 + 1]
                );
                MILO_ASSERT(SUCCEEDED(hr), 0x24E);
                curBuf.unk0[curBuf.unk10 + 1] = 0;
            }
            curBuf.unkc = curBuf.unkc - 1U & 1;
            curBuf.unk10 = curBuf.unk10 - 1U & 1;
        } else if (hr == E_NUI_DEVICE_NOT_CONNECTED) {
            mConnected = false;
        } else if (hr == (HRESULT)0x83010001 && buf == kBufferColor) {
            g_ColorNoFrameDataCnt++;
        }
    }
}
