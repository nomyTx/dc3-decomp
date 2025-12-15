#include "meta_ham/MetagameStats.h"
#include "game/GameMode.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "meta_ham/HamProfile.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/Symbol.h"

MetagameStats::MetagameStats() : unk13c(SystemConfig("stats")) {
    Clear();
    mSaveSizeMethod = SaveSize;
}

BEGIN_HANDLERS(MetagameStats)
    HANDLE_ACTION(increment_count, IncrementCount((CountStatID)_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(
        tally_favorite,
        TallyFavorite((FavoriteStatID)_msg->Int(2), _msg->Int(3), _msg->Int(4))
    )
    HANDLE_EXPR(get_count, GetCount((CountStatID)_msg->Int(2)))
    HANDLE_EXPR(get_favorite, GetFavorite((FavoriteStatID)_msg->Int(2)))
    HANDLE_EXPR(
        get_favorite_count, GetFavoriteCount((FavoriteStatID)_msg->Int(2), _msg->Int(3))
    )
END_HANDLERS

void MetagameStats::SaveFixed(FixedSizeSaveableStream &fs) const {
    for (int i = 0; i < kNumCountStats; i++) {
        fs << unk38[i];
    }
    for (int i = 0; i < kNumFavoriteStats; i++) {
        FixedSizeSaveable::SaveStd(fs, mFavoriteStats[i].mCounts, 1000, 8);
    }
    const_cast<MetagameStats *>(this)->unk144 = false;
}

void MetagameStats::LoadFixed(FixedSizeSaveableStream &fs, int) {
    for (int i = 0; i < kNumCountStats; i++) {
        fs >> unk38[i];
    }
    for (int i = 0; i < kNumFavoriteStats; i++) {
        FixedSizeSaveable::LoadStd(fs, mFavoriteStats[i].mCounts, 1000, 8);
    }
    unk144 = false;
}

int MetagameStats::SaveSize(int) { return 0xDB38; }

void MetagameStats::Clear() {
    unk144 = false;
    for (int i = 0; i < kNumCountStats; i++) {
        unk38[i] = 0;
    }
    for (int i = 0; i < kNumFavoriteStats; i++) {
        mFavoriteStats[i].mCounts.clear();
    }
    unk140 = 0;
}

int MetagameStats::GetCount(CountStatID id) const {
    MILO_ASSERT(id >= 0 && id < kNumCountStats, 0x43);
    return unk38[id];
}

Symbol MetagameStats::SetTitleByThreshold(int i1, DataArray *a2) const {
    Symbol out(gNullStr);
    int i = 1;
    if (a2) {
        int aSize = a2->Size();
        for (; i < aSize; i++) {
            DataArray *curArr = a2->Array(i);
            if (i1 >= curArr->Int(0)) {
                out = curArr->Sym(1);
            }
        }
    }
    return out;
}

int MetagameStats::GetFavorite(FavoriteStatID id) const {
    MILO_ASSERT(id >= 0 && id < kNumFavoriteStats, 0x49);
    int ret = -1;
    int i4 = 0;
    FOREACH (it, mFavoriteStats[id].mCounts) {
        if (it->second > i4) {
            i4 = it->second;
            ret = it->first;
        }
    }
    return ret;
}

int MetagameStats::GetFavoriteCount(FavoriteStatID id, int key) const {
    MILO_ASSERT(id >= 0 && id < kNumFavoriteStats, 0x5E);
    MILO_ASSERT(mFavoriteStats[id].mCounts.find(key) != mFavoriteStats[id].mCounts.end(), 0x5F);
    return mFavoriteStats[id].mCounts.find(key)->second;
}

void MetagameStats::UpdatePartyStats(int x) {
    if (x > unk38[0x13]) {
        unk38[0x13] = x;
        unk144 = true;
    }
}

void MetagameStats::PhotoTaken() {
    unk38[0xe]++;
    unk144 = true;
    static Symbol perform("perform");
    static Symbol dance_battle("dance_battle");
    static Symbol gameplay_mode("gameplay_mode");
    if (TheGameMode->Property(gameplay_mode)->Sym() == perform) {
        unk144 = true;
        unk38[0xf]++;
    }
    if (TheGameMode->Property(gameplay_mode)->Sym() == dance_battle) {
        unk144 = true;
        unk38[0x10]++;
    }
}

void MetagameStats::TallyFavorite(FavoriteStatID id, int i2, int i3) {
    MILO_ASSERT(id >= 0 && id < kNumFavoriteStats, 0x3C);
    mFavoriteStats[id].mCounts[i2] += i3;
    unk144 = true;
}

void MetagameStats::WriteTimePlayed(HamProfile *profile, int i2) {
    unk38[1] += i2;
    unk144 = true;
    static Symbol perform("perform");
    static Symbol practice("practice");
    static Symbol dance_battle("dance_battle");
    static Symbol gameplay_mode("gameplay_mode");
    if (TheGameMode->Property(gameplay_mode)->Sym() == perform) {
        unk144 = true;
        unk38[2] += i2;
        mFavoriteStats[0].mCounts[0] += i2;
        unk144 = true;
    }
    if (TheGameMode->Property(gameplay_mode)->Sym() == practice) {
        unk144 = true;
        unk38[3] += i2;
        mFavoriteStats[0].mCounts[1] += i2;
        unk144 = true;
    }
    if (TheGameMode->Property(gameplay_mode)->Sym() == dance_battle) {
        unk144 = true;
        unk38[4] += i2;
        mFavoriteStats[0].mCounts[2] += i2;
        unk144 = true;
    }
    unk140 += i2;
}

void MetagameStats::IncrementCount(CountStatID id, int cnt) {
    MILO_ASSERT(id >= 0 && id < kNumCountStats, 0x35);
    unk38[id] += cnt;
    unk144 = true;
}
