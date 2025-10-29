#include "os/ContentMgr_Xbox.h"
#include "ContentMgr.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "xdk/xapilibi/xcontentcrosstitle.h"
#include "xdk/xapilibi/xgetoverlappedresult.h"
#include "xdk/xapilibi/xuser.h"

std::vector<String> gIgnoredContent;
XboxContentMgr gContentMgr;
const char *kContentRootFormat = "cnt%08x";

#define kNumberOfBuffers 7
#define kContentRootMaxLength 12

XboxContent::XboxContent(const XCONTENT_CROSS_TITLE_DATA &data, int i2, int i3, bool b4)
    : mOverlapped(0), mLicenseBits(0), mValidLicenseBits(0),
      mRoot(MakeString(kContentRootFormat, i2)), unk150(MakeString("%s:", mRoot.c_str())),
      mState(kUnmounted), mPadNum(i3), unk160(0), unk161(0), unk168(0) {
    MILO_ASSERT(mRoot.size() < kContentRootMaxLength, 0x6F);
    MILO_ASSERT(mPadNum < kNumberOfBuffers, 0x70);
    mXData = data;
    char filename[XCONTENT_MAX_FILENAME_LENGTH + 1];
    memcpy(filename, mXData.szFileName, XCONTENT_MAX_FILENAME_LENGTH);
    filename[XCONTENT_MAX_FILENAME_LENGTH] = 0;
    mFilename = filename;
    mState = (State)(b4 != 0);
}

XboxContent::~XboxContent() {
    switch (mState) {
    case 0:
    case 1:
    case 3:
    case 5:
    case 6:
    case 8:
        break;
    case kMounting:
    case kMounted:
        XContentClose(mRoot.c_str(), nullptr);
        break;
    default:
        int state = mState;
        MILO_LOG("Unknown state: %d", state);
        break;
    }
}

ContentLocT XboxContent::Location() {
    XDEVICE_DATA deviceData;
    XContentGetDeviceData(mXData.DeviceID, &deviceData);
    if (deviceData.DeviceType == 2) {
        return kLocationRemovableMem;
    } else {
        if (deviceData.DeviceType != 1) {
            MILO_NOTIFY(
                "Unknown device type: %d - defaulting to HDD", deviceData.DeviceType
            );
        }
        return kLocationHDD;
    }
}

void XboxContent::Poll() {
    if (mState == 1) {
        MILO_LOG("Mounting content '%s'\n", FileName());
        int pad = mPadNum;
        if (pad == 4 || pad == 5) {
            pad = 0xFF;
        }
        mOverlapped = new XOVERLAPPED();
        if (XContentCrossTitleCreate(
                pad, mRoot.c_str(), &mXData, 3, nullptr, &mLicenseBits, 0, 0, mOverlapped
            )
            != 0x3E5) {
            RELEASE(mOverlapped);
            mState = kContentDeleting;
        } else {
            mState = kMounting;
        }
    }
    if (mState == 2 || mState == 3) {
        DWORD res = XGetOverlappedResult(mOverlapped, nullptr, false);
        if (res == 0x3E4)
            return;
        if (res == 0) {
            mValidLicenseBits = true;
            mState = mState == kMounting ? kMounted : kUnmounted;
            if (unk160) {
                Delete();
            }
        } else {
            unsigned short err = XGetOverlappedExtendedError(mOverlapped);
            mState = kContentDeleting;
            unk161 = err == 0x570;
        }
        RELEASE(mOverlapped);
    }
    if (mState == 6) {
        DWORD res = XGetOverlappedResult(mOverlapped, nullptr, false);
        if (res != 0x3E4) {
            RELEASE(mOverlapped);
            mState = (State)(res == 7);
        }
    }
}

void XboxContent::Mount() {
    if (mState == kUnmounted) {
        mState = kNeedsMounting;
        static unsigned int count = 0;
        count++;
        unk168 = count;
    }
}

void XboxContent::Unmount() {
    if (mState == kMounted) {
        mOverlapped = new XOVERLAPPED();
        if (XContentClose(mRoot.c_str(), mOverlapped) != 0x3E5) {
            RELEASE(mOverlapped);
            mState = kContentDeleting;
            return;
        }
        mState = kUnmounting;
    } else if (mState == kNeedsMounting || mState == kContentDeleting) {
        mState = kUnmounted;
    }
}

void XboxContent::Delete() {
    unk160 = true;
    if (mState == 4 || mState == 1) {
        Unmount();
    } else if (mState == 0) {
        mOverlapped = new XOVERLAPPED();
        if (XContentCrossTitleDelete(0xFF, &mXData, mOverlapped) != 0x3E5) {
            RELEASE(mOverlapped);
            mState = kContentDeleting;
        } else {
            mState = kNeedsBackup;
        }
    }
}
