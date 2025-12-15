#include "meta_ham/MetagameRank.h"
#include "meta/FixedSizeSaveableStream.h"
#include "meta_ham/HamProfile.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "os/System.h"
#include <cstdio>

namespace {
    DataArray *gRanksArray;
    DataArray *gRepeatableTasks;
    DataArray *gOneTimeTasks;

    // size 0x14
    struct DeferredAward {
        String unk0;
        Symbol unk8;
        Symbol unkc;
        Symbol unk10;
    };

    // size 0x20
    struct Unlockable {
        int unk0;
        Symbol unk4;
        Symbol unk8;
        Symbol unkc;
        Symbol unk10;
        std::vector<Symbol> unk14;
    };
    std::vector<Unlockable> gUnlockables;
    std::vector<Unlockable *> gTiers;
    std::list<DeferredAward> gDeferredAwardQueue;
}

MetagameRank::MetagameRank(HamProfile *p) : mProfile(p) {
    Clear();
    mSaveSizeMethod = SaveSize;
}

BEGIN_HANDLERS(MetagameRank)
    HANDLE_EXPR(get_score, mScore)
    HANDLE_EXPR(get_rank_number, mRankNumber)
    HANDLE_EXPR(get_rank_in_tier, GetRankInTier())
    HANDLE_EXPR(get_tier, GetTier())
    HANDLE_EXPR(get_xp_of_rank, GetXPOfRank(_msg->Int(2)))
    HANDLE_EXPR(has_new_rank, HasNewRank())
    HANDLE_EXPR(at_max_rank, mAtMaxRank)
    HANDLE_EXPR(get_percent_to_next_rank, mPctToNextRank)
    HANDLE_ACTION(award_points, AwardPointsForTask(_msg->Sym(2)))
    HANDLE_EXPR(have_deferred_points, mDeferredPoints.size() > 0)
    HANDLE(get_next_deferred_points, GetNextDeferredPoints)
END_HANDLERS

bool MetagameRank::HasNewRank() const {
    if (!mAtMaxRank) {
        return mPctToNextRank == 1;
    } else {
        return unkc9;
    }
}

void MetagameRank::SaveFixed(FixedSizeSaveableStream &fs) const {
    fs << mScore;
    bool b1 = unk38;
    if (!b1) {
        static Symbol play_first_time_disp("play_first_time_disp");
        FOREACH (it, mDeferredPoints) {
            if (it->unk4 == play_first_time_disp) {
                b1 = true;
                break;
            }
        }
    }
    fs << b1;
    fs << mAtMaxRank;
    fs.Write(unk39, 0x40);
    fs.Write(unk79, 0x40);
    static Symbol combined_xp_disp("combined_xp_disp");
    int sum;
    if (mDeferredPoints.size() != 0) {
        sum = 0;
        FOREACH (it, mDeferredPoints) {
            sum += it->unk0;
        }
    } else {
        sum = 0;
    }
    SaveSymbolID(fs, combined_xp_disp);
    fs << sum;
    const_cast<MetagameRank *>(this)->unkca = false;
}

void MetagameRank::LoadFixed(FixedSizeSaveableStream &fs, int i2) {
    fs >> mScore;
    if (i2 > 0x45) {
        fs >> unk38;
    }
    if (i2 > 0x4E) {
        fs >> mAtMaxRank;
    }
    fs.Read(unk39, 0x40);
    if (unk38) {
        int idx = -1;
        static Symbol play_first_time("play_first_time");
        GetOneTimeTask(play_first_time, nullptr, &idx);
        if (idx >= 0) {
            unk39[idx] = 0;
        }
    }
    fs.Read(unk79, 0x40);
    if (i2 > 0x3D) {
        if (i2 <= 0x5A) {
            int x;
            fs >> x;
        }
    }
    if (i2 > 0x5A) {
        DeferredPoints pt;
        LoadSymbolFromID(fs, pt.unk4);
        fs >> pt.unk0;
        if (pt.unk0 > 0) {
            mDeferredPoints.push_back(pt);
        }
    }
    ComputeRankNumber(true);
    unkca = false;
}

void MetagameRank::Preinit() {
    DataArray *rankCfg = SystemConfig("rank");
    gRanksArray = rankCfg->FindArray("ranks");
}

DataNode HaveDeferredAward(DataArray *) { return !gDeferredAwardQueue.empty(); }
DataNode HandleDeferredAward(DataArray *) {
    if (gDeferredAwardQueue.empty()) {
        return 0;
    } else {
        DeferredAward award = gDeferredAwardQueue.front();
        gDeferredAwardQueue.pop_front();
        DataArrayPtr ptr(Symbol(award.unk0.c_str()), award.unk8, award.unkc, award.unk10);
        return ptr;
    }
}

void MetagameRank::Init() {
    static DataNode &xp_force_award_small = DataVariable("xp_force_award_small");
    static DataNode &xp_force_award_medium = DataVariable("xp_force_award_medium");
    static DataNode &xp_force_award_large = DataVariable("xp_force_award_large");
    static DataNode &xp_force_award_one_time = DataVariable("xp_force_award_one_time");
    static DataNode &xp_force_award_all = DataVariable("xp_force_award_all");
    static DataNode &xp_force_one_rank_up = DataVariable("xp_force_one_rank_up");
    xp_force_award_small = 0;
    xp_force_award_medium = 0;
    xp_force_award_large = 0;
    xp_force_award_one_time = 0;
    xp_force_award_all = 0;
    xp_force_one_rank_up = 0;
    DataRegisterFunc("xp_have_deferred_award", HaveDeferredAward);
    DataRegisterFunc("xp_deferred_award", HandleDeferredAward);
    DataArray *rankCfg = SystemConfig("rank");
    DataArray *unlockArr = rankCfg->FindArray("unlockables");
    if (unlockArr) {
        int newSize = unlockArr->Size() - 1;
        gUnlockables.resize(newSize);
        for (int i = 0; i < newSize; i++) {
            DataArray *curUnlockArray = unlockArr->Array(i + 1);
            Unlockable &cur = gUnlockables[i];
            cur.unk0 = i + 1;
            cur.unk4 = curUnlockArray->Sym(0);
            cur.unk8 = curUnlockArray->FindSym("name");
            cur.unkc = curUnlockArray->FindSym("desc");
            cur.unk10 = curUnlockArray->FindSym("image");
            DataArray *unlocksToPopulate = curUnlockArray->FindArray("unlock");
            cur.unk14.resize(unlocksToPopulate->Size() - 1);
            for (int j = 1; j < unlocksToPopulate->Size(); j++) {
                cur.unk14[j - 1] = unlocksToPopulate->Sym(j);
                // TheAccomplishmentManager AddAssetAward
            }
        }
    }
    DataArray *tierArr = rankCfg->FindArray("tiers");
    if (tierArr) {
        int newSize = tierArr->Size() - 1;
        gTiers.resize(newSize);
        for (int i = 0; i < newSize; i++) {
            DataArray *innerTierArr = tierArr->FindArray(i + 1);
            int innerSize = innerTierArr->Size();
            gTiers.reserve(innerSize);
            for (int j = 0; j < innerSize; j++) {
                bool b3 = false;
                Symbol s128 = innerTierArr->Sym(j);
                // there's more
            }
        }
    }
    DataArray *taskArr = rankCfg->FindArray("tasks");
    gRepeatableTasks = taskArr->FindArray("repeatable");
    gOneTimeTasks = rankCfg->FindArray("one_time");
}

void MetagameRank::Clear() {
    unkca = false;
    mScore = 0;
    unk38 = true;
    memset(unk39, 0, 0x40);
    memset(unk79, 0, 0x40);
    mDeferredPoints.clear();
    mRankNumber = 0;
    mAtMaxRank = false;
    unkc9 = false;
    mPctToNextRank = 0;
    ComputeRankNumber(true);
}

Symbol MetagameRank::GetRankTitle() const {
    char buf[32];
    sprintf(buf, "rank_%d", mRankNumber);
    return buf;
}

bool MetagameRank::GetOneTimeTask(Symbol s, DataArray **aptr, int *iptr) {
    if (aptr || iptr) {
        for (int i = 1; i < gOneTimeTasks->Size(); i++) {
            DataNode &n = gOneTimeTasks->Node(i);
            if (n.Type() == kDataArray) {
                if (n.Array()->Sym(0) == s) {
                    if (aptr) {
                        *aptr = n.Array();
                    }
                    if (iptr) {
                        *iptr = i - 1;
                    }
                    return true;
                }
            }
        }
        if (aptr) {
            *aptr = nullptr;
        }
        if (iptr) {
            *iptr = -1;
        }
    }
    return false;
}

int MetagameRank::GetXPOfRank(int i) const {
    int ret = 0;
    if (i > 0 && i < gRanksArray->Size() - 1) {
        ret = gRanksArray->Array(i + 1)->Int(0);
        if (i >= 1) {
            ret -= gRanksArray->Array(i)->Int(0);
        }
    }
    return ret;
}

DataNode MetagameRank::GetNextDeferredPoints(DataArray *a) {
    if (mDeferredPoints.empty()) {
        static Symbol xp_previous_points_msg("xp_previous_points_msg");
        return xp_previous_points_msg;
    } else {
        DeferredPoints pt = mDeferredPoints.front();
        mDeferredPoints.pop_front();
        mScore += pt.unk0;
        ComputeRankNumber(false);
        unkca = true;
        DataArrayPtr ptr(pt.unk4, pt.unk0);
        return ptr;
    }
}
