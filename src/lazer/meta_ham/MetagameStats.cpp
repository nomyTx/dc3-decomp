#include "meta_ham/MetagameStats.h"
#include "game/GameMode.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/Utl.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "ui/UIListLabel.h"
#include "ui/UIListMesh.h"
#include "utl/Locale.h"
#include "utl/Symbol.h"

MetagameStats::MetagameStats() : mStatsCfg(SystemConfig("stats")) {
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

void MetagameStats::Text(int, int data, UIListLabel *slot, UILabel *label) const {
    MILO_ASSERT(data >= 0 && data < NumData(), 0x69);
    DataArray *statArr = mStatsCfg->Array(data + 1);
    if (statArr->Int(0) == -1) {
        if (slot->Matches("header")) {
            label->SetTextToken(statArr->Sym(1));
        }
    } else if (statArr->Int(0) != -2) {
        if (slot->Matches("stat") || slot->Matches("title")) {
            int id = statArr->Int(0); // count/fave stat id
            StatType statType = (StatType)statArr->Int(1); // stat type
            Symbol statSym = statArr->Sym(2); // the actual stat symbol
            DataArray *titleArr = statArr->FindArray("titles", false);
            Symbol scc(gNullStr);
            switch (statType) {
            case kStatType_Count: {
                int count = GetCount((CountStatID)id);
                if (slot->Matches("title")) {
                    Symbol title = SetTitleByThreshold(count, titleArr);
                    label->SetTextToken(title);
                } else {
                    const char *loc = LocalizeSeparatedInt(count, TheLocale);
                    label->SetTokenFmt(statSym, loc);
                }
                break;
            }
            case kStatType_Time: {
                int count = GetCount((CountStatID)id);
                if (slot->Matches("title")) {
                    Symbol title = SetTitleByThreshold(count, titleArr);
                    label->SetTextToken(title);
                } else {
                    char buf[32];
                    GetTimeString(count, buf);
                    label->SetTokenFmt(statSym, buf);
                }
                break;
            }
            case kStatType_FavoriteMode: {
                static Symbol stats_favorite_na("stats_favorite_na");
                static Symbol perform("perform");
                static Symbol practice("practice");
                static Symbol dancebattle("dancebattle");
                int fave = GetFavorite((FavoriteStatID)id);
                if (slot->Matches("title")) {
                    Symbol title = SetTitleForFavorite(fave, 0, nullptr);
                    label->SetTextToken(title);
                } else {
                    Symbol tag = stats_favorite_na;
                    if (fave == 0) {
                        tag = perform;
                    } else if (fave == 1) {
                        tag = practice;
                    } else if (fave == 2) {
                        tag = dancebattle;
                    }
                    label->SetTokenFmt(statSym, Localize(tag, nullptr, TheLocale));
                }
                break;
            }
            case kStatType_FavoriteSong: {
                static Symbol stats_favorite_na("stats_favorite_na");
                int fave = GetFavorite((FavoriteStatID)id);
                int faveCount = 0;
                const char *loc = Localize(stats_favorite_na, nullptr, TheLocale);
                if (fave != -1 && TheHamSongMgr.Data(fave)) {
                    loc = TheHamSongMgr.Data(fave)->Title();
                    faveCount = GetFavoriteCount((FavoriteStatID)id, fave);
                }
                if (slot->Matches("title")) {
                    Symbol title = SetTitleForFavorite(fave, faveCount, statArr);
                    label->SetTextToken(title);
                } else if (faveCount == 1) {
                    Symbol single = MakeString("%s_single", statSym.Str());
                    label->SetTokenFmt(single, loc);
                } else {
                    label->SetTokenFmt(
                        statSym, loc, LocalizeSeparatedInt(faveCount, TheLocale)
                    );
                }
                break;
            }
            case kStatType_FavoriteCharacter: {
                static Symbol stats_favorite_na("stats_favorite_na");
                int fave = GetFavorite((FavoriteStatID)id);
                Symbol s15 = stats_favorite_na;
                if (fave != -1) {
                    DataArray *characterArray =
                        SystemConfig()->FindArray("characters", false);
                    MILO_ASSERT(characterArray, 0xDF);
                    for (int i = 1; i < characterArray->Size(); i++) {
                        if (fave == i) {
                            DataArray *characterEntry = characterArray->Array(i);
                            MILO_ASSERT(characterEntry, 0xE5);
                            s15 = characterEntry->Sym(0);
                        }
                    }
                }
                if (slot->Matches("title")) {
                    Symbol title = SetTitleForFavorite(fave, 0, statArr);
                    label->SetTextToken(title);
                } else {
                    label->SetTokenFmt(statSym, s15);
                }
                break;
            }
            default:
                break;
            }
        }
    }
}

RndMat *MetagameStats::Mat(int, int data, UIListMesh *slot) const {
    MILO_ASSERT(data >= 0 && data < NumData(), 0xFC);
    DataArray *arr = mStatsCfg->Array(data + 1);
    int i1 = arr->Int(0);
    if (i1 == -1 && slot->Matches("underline")) {
        return slot->DefaultMat();
    } else {
        return nullptr;
    }
}

void MetagameStats::SaveFixed(FixedSizeSaveableStream &fs) const {
    for (int i = 0; i < kNumCountStats; i++) {
        fs << mCountStats[i];
    }
    for (int i = 0; i < kNumFavoriteStats; i++) {
        FixedSizeSaveable::SaveStd(fs, mFavoriteStats[i].mCounts, 1000, 8);
    }
    const_cast<MetagameStats *>(this)->mDirty = false;
}

void MetagameStats::LoadFixed(FixedSizeSaveableStream &fs, int) {
    for (int i = 0; i < kNumCountStats; i++) {
        fs >> mCountStats[i];
    }
    for (int i = 0; i < kNumFavoriteStats; i++) {
        FixedSizeSaveable::LoadStd(fs, mFavoriteStats[i].mCounts, 1000, 8);
    }
    mDirty = false;
}

int MetagameStats::SaveSize(int) { return 0xDB38; }

void MetagameStats::Clear() {
    mDirty = false;
    for (int i = 0; i < kNumCountStats; i++) {
        mCountStats[i] = 0;
    }
    for (int i = 0; i < kNumFavoriteStats; i++) {
        mFavoriteStats[i].mCounts.clear();
    }
    unk140 = 0;
}

int MetagameStats::GetCount(CountStatID id) const {
    MILO_ASSERT(id >= 0 && id < kNumCountStats, 0x43);
    return mCountStats[id];
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
    if (x > mCountStats[kCountStat_LongestPartyAttended]) {
        mCountStats[kCountStat_LongestPartyAttended] = x;
        mDirty = true;
    }
}

void MetagameStats::PhotoTaken() {
    mCountStats[kCountStat_PhotosTaken]++;
    mDirty = true;
    static Symbol perform("perform");
    static Symbol dance_battle("dance_battle");
    static Symbol gameplay_mode("gameplay_mode");
    if (TheGameMode->Property(gameplay_mode)->Sym() == perform) {
        mDirty = true;
        mCountStats[kCountStat_PhotosTakenPerform]++;
    }
    if (TheGameMode->Property(gameplay_mode)->Sym() == dance_battle) {
        mDirty = true;
        mCountStats[kCountStat_PhotosTakenMultiplayer]++;
    }
}

void MetagameStats::TallyFavorite(FavoriteStatID id, int i2, int i3) {
    MILO_ASSERT(id >= 0 && id < kNumFavoriteStats, 0x3C);
    mFavoriteStats[id].mCounts[i2] += i3;
    mDirty = true;
}

void MetagameStats::WriteTimePlayed(HamProfile *profile, int i2) {
    mCountStats[kCountStat_TotalTimePlayed] += i2;
    mDirty = true;
    static Symbol perform("perform");
    static Symbol practice("practice");
    static Symbol dance_battle("dance_battle");
    static Symbol gameplay_mode("gameplay_mode");
    if (TheGameMode->Property(gameplay_mode)->Sym() == perform) {
        mDirty = true;
        mCountStats[kCountStat_TotalTimePerforming] += i2;
        mFavoriteStats[kFavoriteStat_FavoriteMode].mCounts[0] += i2;
        mDirty = true;
    }
    if (TheGameMode->Property(gameplay_mode)->Sym() == practice) {
        mDirty = true;
        mCountStats[kCountStat_TotalTimeRehearsing] += i2;
        mFavoriteStats[kFavoriteStat_FavoriteMode].mCounts[1] += i2;
        mDirty = true;
    }
    if (TheGameMode->Property(gameplay_mode)->Sym() == dance_battle) {
        mDirty = true;
        mCountStats[kCountStat_TotalTimeMultiplayer] += i2;
        mFavoriteStats[kFavoriteStat_FavoriteMode].mCounts[2] += i2;
        mDirty = true;
    }
    unk140 += i2;
}

void MetagameStats::IncrementCount(CountStatID id, int cnt) {
    MILO_ASSERT(id >= 0 && id < kNumCountStats, 0x35);
    mCountStats[id] += cnt;
    mDirty = true;
}

Symbol MetagameStats::SetTitleForFavorite(int i1, int i2, DataArray *a3) const {
    Symbol ret(gNullStr);
    if (a3) {
        int aSize = a3->Size();
        for (int i = 1; i < aSize; i++) {
            DataArray *curArr = a3->Array(i);
            int i0 = curArr->Int(0);
            if (i1 == i0) {
                if (curArr->Size() == 2) {
                    ret = curArr->Sym(1);
                } else {
                    if (i2 < curArr->Int(1)) {
                        continue;
                    }
                    ret = curArr->Sym(2);
                }
            }
        }
    }
    return ret;
}

bool MetagameStats::InqStatString(int statIndex, String &statStr) {
    MILO_ASSERT_RANGE(statIndex, 0, NumData(), 0x10B);
    DataArray *stat = mStatsCfg->Array(statIndex + 1);
    MILO_ASSERT(stat, 0x10D);
    int id = stat->Int(0);
    StatType statType = (StatType)stat->Int(1);
    Symbol statSym = stat->Sym(2);
    switch (statType) {
    case kStatType_Count: {
        statStr = MakeString(
            Localize(statSym, nullptr, TheLocale),
            LocalizeSeparatedInt(GetCount((CountStatID)id), TheLocale)
        );
        break;
    }
    case kStatType_Time: {
        char buf[32];
        int count = GetCount((CountStatID)id);
        GetTimeString(count, buf);
        statStr = MakeString(Localize(statSym, nullptr, TheLocale), buf);
        break;
    }
    case kStatType_FavoriteMode: {
        static Symbol stats_favorite_na("stats_favorite_na");
        static Symbol perform("perform");
        static Symbol practice("practice");
        static Symbol dancebattle("dancebattle");
        int fave = GetFavorite((FavoriteStatID)id);
        Symbol tag = stats_favorite_na;
        if (fave == 0) {
            tag = perform;
        } else if (fave == 1) {
            tag = practice;
        } else if (fave == 2) {
            tag = dancebattle;
        }
        statStr = MakeString(Localize(statSym, nullptr, TheLocale), tag);
        break;
    }
    case kStatType_FavoriteSong: {
        static Symbol stats_favorite_na("stats_favorite_na");
        int fave = GetFavorite((FavoriteStatID)id);
        int faveCount = 0;
        const char *localized = Localize(stats_favorite_na, nullptr, TheLocale);
        if (fave != -1 && TheHamSongMgr.Data(fave)) {
            localized = TheHamSongMgr.Data(fave)->Title();
            faveCount = GetFavoriteCount((FavoriteStatID)id, fave);
            if (faveCount == 1) {
                Symbol single = MakeString("%s_single", statSym.Str());
                statStr = MakeString(Localize(single, nullptr, TheLocale), localized);
                return true;
            }
        }
        statStr = MakeString(
            Localize(statSym, nullptr, TheLocale),
            localized,
            LocalizeSeparatedInt(faveCount, TheLocale)
        );
        break;
    }
    case kStatType_FavoriteCharacter: {
        static Symbol stats_favorite_na("stats_favorite_na");
        int fave = GetFavorite((FavoriteStatID)id);
        Symbol s9 = stats_favorite_na;
        if (fave != -1) {
            DataArray *characterArray = SystemConfig()->FindArray("characters", false);
            MILO_ASSERT(characterArray, 0x155);
            for (int i = 1; i < characterArray->Size(); i++) {
                if (fave == i) {
                    DataArray *characterEntry = characterArray->Array(i);
                    MILO_ASSERT(characterEntry, 0x15B);
                    s9 = MakeString("%s_title", characterEntry->Sym(0).Str());
                }
            }
        }
        statStr = MakeString(
            Localize(statSym, nullptr, TheLocale), Localize(s9, nullptr, TheLocale)
        );
        break;
    }
    default:
        break;
    }
    return true;
}
