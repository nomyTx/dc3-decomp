#include "MemcardMgr.h"
#include "meta/MemcardAction.h"
#include "meta/MemcardMgr.h"
#include "meta/Profile.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/Memcard.h"
#include "os/Memcard_Xbox.h"
#include "os/PlatformMgr.h"
#include "os/ThreadCall.h"
#include "utl/Locale.h"
#include "xdk/xapilibi/stringapiset.h"

namespace {
    const char *kSaveFilename = "save.dat";
}

MemcardMgr::MemcardMgr()
    : mState(kS_None), mAction(0), unk40(0), unk88(-1), mSelectDeviceWaiting(0),
      mSelectDeviceCallBackObj(0), mPadNum(-1), mProfile(0) {}

MemcardMgr::~MemcardMgr() {}

BEGIN_HANDLERS(MemcardMgr)
    HANDLE_MESSAGE(DeviceChosenMsg)
    HANDLE_MESSAGE(NoDeviceChosenMsg)
    HANDLE_MESSAGE(UIChangedMsg)
    HANDLE_MESSAGE(StorageChangedMsg)
    HANDLE_MESSAGE(SigninChangedMsg)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void MemcardMgr::Init() {
    SetName("memcardmgr", ObjectDir::Main());
    TheMC.SetContainerName("savegame");
    static Symbol title_name("title_name");
    for (int i = 0; i < 4; i++) {
        mValidDevices[i] = false;
        unk74[i] = 0;
        mContainerIDs[i].Set(i, 0);
    }
    WCHAR wideName[128];
    MultiByteToWideChar(
        0, 0, Localize(title_name, nullptr, TheLocale), -1, wideName, DIM(wideName)
    );
    TheMC.SetContainerDisplayName(wideName);
    ThePlatformMgr.AddSink(this);
}

int MemcardMgr::ThreadStart() {
    int ret = 0;
    switch (mState) {
    case kS_None:
        MILO_FAIL("ThreadCall triggered MemcardMgr::ThreadStart with no mode set.\n");
        break;
    case kS_Search:
        ret = ThreadCall_SearchForDevice();
        break;
    case kS_CheckForSaveContainer:
        ret = ThreadCall_CheckForSaveContainer();
        break;
    case kS_SaveGame:
        ret = ThreadCall_SaveGame();
        break;
    case kS_LoadGame:
        ret = ThreadCall_LoadGame();
        break;
    case kS_DeleteSaves:
        ret = TheMC.DeleteContainer(mContainerIDs[mPadNum]);
        break;
    }
    return ret;
}

void MemcardMgr::ThreadDone(int mcResult) {
    State oldState = mState;
    mState = kS_None;
    switch (oldState) {
    case kS_None:
        MILO_FAIL("ThreadCall triggered MemcardMgr::ThreadDone with no mode set.\n");
        break;
    case kS_Search:
    case kS_CheckForSaveContainer: {
        MCResultMsg msg((MCResult)mcResult);
        Export(msg, true);
        break;
    }
    case kS_SaveGame:
    case kS_LoadGame: {
        mAction->SetResult((MCResult)mcResult);
        mAction->PostAction();
        if (mcResult == 0) {
            MCResult res = mAction->Result();
            if (res != kMCNoError) {
                mcResult = res;
            }
        }
        mAction = nullptr;
        mPadNum = -1;
        MCResultMsg msg((MCResult)mcResult);
        Export(msg, true);
        break;
    }
    case kS_DeleteSaves: {
        mPadNum = -1;
        MCResultMsg msg((MCResult)mcResult);
        Export(msg, true);
        break;
    }
    default:
        break;
    }
}

bool MemcardMgr::IsStorageDeviceValid(Profile *pProfile) {
    MILO_ASSERT(pProfile, 0x122);
    int padNum = pProfile->GetPadNum();
    MILO_ASSERT(padNum != -1, 0x124);
    if (mValidDevices[padNum]) {
        mValidDevices[padNum] = TheMC.IsDeviceValid(mContainerIDs[padNum]);
    }
    return mValidDevices[padNum];
}

void MemcardMgr::OnCheckForSaveContainer(Profile *pProfile) {
    MILO_ASSERT(pProfile, 0x1BF);
    mProfile = pProfile;
    mPadNum = mProfile->GetPadNum();
    MILO_ASSERT(mState == kS_None, 0x1C3);
    mState = kS_CheckForSaveContainer;
    ThreadCall(this);
}

void MemcardMgr::OnDeleteSaves(Profile *pProfile) {
    MILO_ASSERT(pProfile, 0x2EF);
    mProfile = pProfile;
    mPadNum = mProfile->GetPadNum();
    MILO_ASSERT(mState == kS_None, 0x2F4);
    mState = kS_DeleteSaves;
    ThreadCall(this);
}

void MemcardMgr::OnLoadGame(Profile *pProfile, MemcardAction *pAction) {
    MILO_ASSERT(pProfile, 0x2D9);
    mProfile = pProfile;
    mPadNum = mProfile->GetPadNum();
    mAction = pAction;
    MILO_ASSERT(mAction, 0x2DF);
    mAction->PreAction();
    MILO_ASSERT(mState == kS_None, 0x2E2);
    mState = kS_LoadGame;
    ThreadCall(this);
}

void MemcardMgr::OnSaveGame(Profile *pProfile, MemcardAction *pAction, int i3) {
    MILO_ASSERT(pProfile, 0x26D);
    unk40 = i3;
    mProfile = pProfile;
    mPadNum = mProfile->GetPadNum();
    mAction = pAction;
    MILO_ASSERT(mAction, 0x274);
    mAction->PreAction();
    MCResult res = mAction->Result();
    if (res != kMCNoError) {
        MCResultMsg msg(res);
        Export(msg, true);
    } else {
        MILO_ASSERT(mState == kS_None, 0x281);
        mState = kS_SaveGame;
        ThreadCall(this);
    }
}

void MemcardMgr::OnSearchForDevice(Profile *pProfile) {
    MILO_ASSERT(pProfile, 0xB5);
    mProfile = pProfile;
    mPadNum = mProfile->GetPadNum();
    MILO_ASSERT(mState == kS_None, 0xBA);
    mState = kS_Search;
    ThreadCall(this);
}

void MemcardMgr::SelectDevice(
    Profile *pProfile, Hmx::Object *callbackObj, int i3, bool waiting
) {
    MILO_ASSERT(mSelectDeviceWaiting == false, 0x82);
    MILO_ASSERT(mSelectDeviceCallBackObj == NULL, 0x83);
    MILO_ASSERT(pProfile, 0x84);
    mProfile = pProfile;
    mSelectDeviceCallBackObj = callbackObj;
    mPadNum = mProfile->GetPadNum();
    if (ThePlatformMgr.GuideShowing()) {
        unk88 = i3;
        mSelectDeviceWaiting = true;
    } else {
        TheMC.ShowDeviceSelector(mContainerIDs[mPadNum], this, i3, waiting);
    }
}
