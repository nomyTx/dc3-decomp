#pragma once
#include "MemStats.h"
#include "obj/Data.h"
#include "utl/AllocInfo.h"
#include "utl/KeylessHash.h"
#include "utl/Str.h"
#include "utl/TextFileStream.h"
#include "utl/TextStream.h"

// size 0x1820c
class MemTracker {
public:
    MemTracker(int, int);
    const AllocInfo *GetInfo(void *) const;
    void Alloc(
        int, int, const char *, void *, signed char, bool, unsigned char, const char *, int
    );
    void Free(void *);
    void CloseReport();
    void SetAllocInfoName(const char *);
    void StartLog(TextStream &);
    void StopLog();
    void Realloc(void *, int, int, void *);
    void HeapReport(TextStream &);
    void DiffDump(TextStream &);

    static void *operator new(unsigned int);
    static void operator delete(void *);
    static int SpitAllocInfo(TextStream *);

private:
    void UpdateStats();
    void ColatedPrint(TextStream &, AllocInfo *, const char *);

    static DataNode SpitAllocInfo(DataArray *);

    void *mHashMem; // 0x0
    KeylessHash<void *, AllocInfo *> *mHashTable; // 0x4
    short mTimeSlice; // 0x8
    HeapStats mHeapStats[16]; // 0xc
    BlockStatTable mMemTable[2]; // 0x14c
    BlockStatTable mPoolTable[2]; // 0xc164
    int mCurStatTable; // 0x1817c
    AllocInfoVec mFreedInfos; // 0x18180
    TextStream *mLog; // 0x1818c
    TextFileStream *unk18190; // 0x18190
    signed char mHeap; // 0x18194
    bool unk18195;
    int mFreeSysMem; // 0x18198
    int mFreePhysMem; // 0x1819c
    int unk181a0;
    String unk181a4;
    String unk181ac;
    String unk181b4;
    char mAllocInfoName[64]; // 0x181bc
};

void MemTrackInit(int, int, bool);
bool MemTrackEnable(bool);
void MemTrackSpew(bool);
void MemTrackSetReportName(const char *);
