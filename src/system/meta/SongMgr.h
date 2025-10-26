#pragma once
#include "meta/SongMetadata.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "utl/BufStream.h"
#include "utl/Cache.h"
#include "utl/MemStream.h"
#include "utl/SongInfoCopy.h"
#include <set>
#include <vector>
#include <map>

// from RB2 taken from RB3 decomp
enum SongMgrState {
    kSongMgr_SaveMount = 0,
    kSongMgr_SaveWrite = 1,
    kSongMgr_SaveUnmount = 2,
    kSongMgr_Ready = 3,
    kSongMgr_Failure = 4,
    kSongMgr_Max = 5,
    kSongMgr_Nil = -1,
};

class SongMgr : public Hmx::Object, public ContentMgr::Callback {
public:
    SongMgr() {}
    // Hmx::Object
    virtual ~SongMgr();
    virtual DataNode Handle(DataArray *, bool);

    // ContentMgr::Callback
    virtual void ContentStarted();
    virtual bool ContentDiscovered(Symbol);
    virtual void ContentMounted(char const *, char const *);
    virtual void ContentUnmounted(char const *);
    virtual void ContentLoaded(Loader *, ContentLocT, Symbol);
    virtual void ContentDone();

    // SongMgr
    virtual void Init();
    virtual void Terminate() {}
    virtual const SongMetadata *Data(int) const;
    virtual SongInfo *SongAudioData(int) const = 0;
    virtual char const *AlternateSongDir() const { return "songs/updates/"; }
    virtual void GetContentNames(Symbol, std::vector<Symbol> &) const;
    virtual bool SongCacheNeedsWrite() const { return mSongCacheNeedsWrite; }
    virtual void ClearSongCacheNeedsWrite() { mSongCacheNeedsWrite = false; }
    virtual void ClearCachedContent(void);
    virtual Symbol GetShortNameFromSongID(int, bool) const = 0;
    virtual int GetSongIDFromShortName(Symbol, bool) const = 0;

    SongInfo *SongAudioData(Symbol) const;
    bool IsSongCacheWriteDone() const;
    char const *GetCachedSongInfoName() const;
    char const *SongPath(Symbol, int) const;
    char const *SongFilePath(Symbol, char const *, int) const;
    void DumpSongMgrContents(bool);
    bool HasSong(int) const;
    bool HasSong(Symbol, bool) const;
    int GetCachedSongInfoSize(void) const;
    bool IsSongMounted(Symbol) const;
    bool SaveCachedSongInfo(BufStream &);
    bool IsContentUsedForSong(Symbol, int) const;
    void StartSongCacheWrite();
    void ClearFromCache(Symbol);
    char const *ContentName(int) const;
    char const *ContentName(Symbol, bool) const;
    bool LoadCachedSongInfo(BufStream &);
    bool SongIDInContent(Symbol key) {
        return mSongIDsInContent.find(key) != mSongIDsInContent.end();
    }

protected:
    virtual bool AllowContentToBeAdded(DataArray *, ContentLocT) { return true; }
    virtual void AddSongData(DataArray *, DataLoader *, ContentLocT) = 0;
    virtual void
    AddSongData(DataArray *, std::map<int, SongMetadata *> &, const char *, ContentLocT, std::vector<int> &) = 0;
    virtual void AddSongIDMapping(int, Symbol) = 0;
    virtual void ReadCachedMetadataFromStream(BinStream &, int) = 0;
    virtual void WriteCachedMetadataToStream(BinStream &) const = 0;

    char const *CachedPath(Symbol, char const *, int) const;
    void SaveMount();
    void SaveUnmount();
    void SaveWrite();
    void GetSongsInContent(Symbol, std::vector<int> &) const;
    char const *ContentNameRoot(Symbol) const;
    int NumSongsInContent(Symbol) const;
    void SetState(SongMgrState);
    void OnCacheMountResult(int);
    void OnCacheWriteResult(int);
    void OnCacheUnmountResult(int);
    void CacheSongData(DataArray *, DataLoader *, ContentLocT, Symbol);

    std::set<int> mAvailableSongs; // 0x30
    std::map<int, SongMetadata *> mUncachedSongMetadata; // 0x48
    SongMgrState mState; // 0x60
    std::map<int, SongMetadata *> mCachedSongMetadata; // 0x64
    std::map<Symbol, std::vector<int> > mSongIDsInContent; // 0x7c
    std::map<int, Symbol> mContentUsedForSong; // 0x94
    std::map<Symbol, String> unkmap5; // 0xac
    CacheID *mSongCacheID; // 0xc4
    Cache *mSongCache; // 0xc8
    bool unkcc; // 0xcc
    bool mSongCacheNeedsWrite; // 0xcd
    bool mSongCacheWriteAllowed; // 0xce
};

int GetSongID(DataArray *, DataArray *);
int CountSongsInArray(DataArray *);

extern SongMgr *TheBaseSongManager;
