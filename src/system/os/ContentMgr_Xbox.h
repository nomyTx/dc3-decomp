#pragma once
#include "os/ContentMgr.h"
#include "utl/UTF8.h"
#include "xdk/XAPILIB.h"

class XboxContent : public Content {
public:
    XboxContent(const XCONTENT_CROSS_TITLE_DATA &, int, int, bool);
    virtual ~XboxContent();
    virtual const char *Root() { return unk150.c_str(); }
    virtual bool OnMemcard() { return Location() == kLocationRemovableMem; }
    virtual ContentLocT Location();
    virtual unsigned long LicenseBits() { return mLicenseBits; }
    virtual bool HasValidLicenseBits() { return mValidLicenseBits; }
    virtual bool IsCorrupt() { return mState == 8 && unk161; }
    virtual State GetState() { return mState; }
    virtual void Poll();
    virtual void Mount();
    virtual void Unmount();
    virtual void Delete();
    virtual Symbol FileName() { return mFilename; }
    virtual const char *DisplayName() {
        const unsigned short *displayName =
            reinterpret_cast<const unsigned short *>(mXData.szDisplayName);
        return WideCharToChar(displayName);
    }
    virtual unsigned int GetLRM() { return unk168; }

private:
    XOVERLAPPED *mOverlapped; // 0x4
    XCONTENT_CROSS_TITLE_DATA mXData; // 0x8
    unsigned long mLicenseBits; // 0x140
    bool mValidLicenseBits; // 0x144
    String mRoot; // 0x148
    String unk150; // 0x150
    State mState; // 0x158
    int mPadNum; // 0x15c
    bool unk160; // 0x160
    bool unk161; // 0x161
    Symbol mFilename; // 0x164
    unsigned int unk168; // 0x168
};

class XboxContentMgr : public ContentMgr {
public:
    // Hmx::Object
    XboxContentMgr() {}
    virtual DataNode Handle(DataArray *, bool);
    // ContentMgr
    virtual void Init();
    virtual void Terminate();
    virtual void StartRefresh();
    virtual void PollRefresh();
    virtual const char *TitleContentPath();
    virtual const char *ContentPath(int);
    virtual bool MountContent(Symbol);
    virtual bool IsMounted(Symbol);
    virtual bool IsCorrupt(Symbol, const char *&);
    virtual bool DeleteContent(Symbol);
    virtual bool IsDeleteDone(Symbol);
    virtual bool GetLicenseBits(Symbol, unsigned long &ul);
    virtual void NotifyMounted(Content *);
    virtual void NotifyUnmounted(Content *);
    virtual void NotifyDeleted(Content *);
    virtual void NotifyFailed(Content *);
};

extern XboxContentMgr gContentMgr;
