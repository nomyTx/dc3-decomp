#include "hamobj/SongCollision.h"
#include "SongCollision.h"
#include "hamobj/Difficulty.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/DataUtl.h"
#include "obj/Object.h"
#include "utl/BinStream.h"
#include "utl/Std.h"

std::vector<const char *> sCollisionUsefulBoneNames;

#pragma region BeatCollisionData

void BeatCollisionData::Set(
    float minX, float minY, const Transform &start_xfm, const Transform &end_xfm
) {
    using namespace Hmx;
    MILO_ASSERT(start_xfm.m == Matrix3::GetIdentity(), 0x62);
    MILO_ASSERT(end_xfm.m == Matrix3::GetIdentity(), 0x63);
    mMinX = minX;
    mMaxX = minY;
    Subtract(start_xfm.v, end_xfm.v, mOffset);
}

BinStream &operator<<(BinStream &bs, const BeatCollisionData &bcd) {
    bs << bcd.mMinX << bcd.mMaxX;
    bs << bcd.mOffset;
    return bs;
}

BinStreamRev &operator>>(BinStreamRev &d, BeatCollisionData &bcd) {
    d >> bcd.mMinX;
    d >> bcd.mMaxX;
    if (d.rev > 1) {
        d >> bcd.mOffset;
    } else if (d.rev > 0) {
        Transform xfm;
        d >> xfm;
        bcd.mOffset = xfm.v;
    }
    return d;
}

#pragma endregion
#pragma region SongCollision

SongCollision::SongCollision() {}

BEGIN_HANDLERS(SongCollision)
    HANDLE_ACTION(update, Update(_msg->Obj<MoveDir>(2)))
    HANDLE_ACTION(print, Print())
    HANDLE_EXPR(equals, Equals(_msg->Obj<SongCollision>(2)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(SongCollision)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(SongCollision)
    SAVE_REVS(2, 1)
    SAVE_SUPERCLASS(Hmx::Object)
    for (int i = 0; i < kNumDifficulties; i++) {
        bs << mData[i];
    }
END_SAVES

BEGIN_COPYS(SongCollision)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(SongCollision)
    BEGIN_COPYING_MEMBERS
        for (int i = 0; i < kNumDifficulties; i++) {
            COPY_MEMBER(mData[i])
        }
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(SongCollision)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 1)
    LOAD_SUPERCLASS(Hmx::Object)
    if (d.rev < 1) {
        for (int i = 0; i < kNumDifficultiesDC2; i++) {
            d >> mData[i];
        }
        mData[kDifficultyBeginner] = mData[kDifficultyEasy];
    } else {
        for (int i = 0; i < kNumDifficulties; i++) {
            d >> mData[i];
        }
    }
END_LOADS

void SongCollision::Print() {
    int maxDatas = 0;
    for (int i = 0; i < kNumDifficulties; i++) {
        maxDatas = Max<int>(maxDatas, mData[i].size());
    }
    String str;
    str = "Beat\tData";
    for (Difficulty d = EasiestDifficulty(); d < kNumDifficulties;
         d = DifficultyOneHarder(d)) {
        str += MakeString("\t%s", DifficultyToSym(d).Str());
    }
    MILO_LOG("%s\n", str.c_str());
    for (int i = 0; i < maxDatas; i++) {
        BeatCollisionData allDatas[kNumDifficulties];
        BeatCollisionData *dataIt = allDatas;
        for (int j = 0; j < kNumDifficulties; j++, dataIt++) {
            if (i < mData[j].size()) {
                *dataIt = mData[j][i];
            } else {
                memset(dataIt, 0, sizeof(BeatCollisionData));
            }
        }
        str = MakeString("%d\tMin X", i);
        for (Difficulty d = EasiestDifficulty(); d < kNumDifficulties;
             d = DifficultyOneHarder(d)) {
            str += MakeString("\t%f", allDatas[d].mMinX);
        }
        MILO_LOG("%s\n", str.c_str());
        str = "\tMax X";
        for (Difficulty d = EasiestDifficulty(); d < kNumDifficulties;
             d = DifficultyOneHarder(d)) {
            str += MakeString("\t%f", allDatas[d].mMaxX);
        }
        MILO_LOG("%s\n", str.c_str());
        str = "\tOffset X";
        for (Difficulty d = EasiestDifficulty(); d < kNumDifficulties;
             d = DifficultyOneHarder(d)) {
            str += MakeString("\t%f", allDatas[d].mOffset.x);
        }
        MILO_LOG("%s\n", str.c_str());
        str = "\tOffset Y";
        for (Difficulty d = EasiestDifficulty(); d < kNumDifficulties;
             d = DifficultyOneHarder(d)) {
            str += MakeString("\t%f", allDatas[d].mOffset.y);
        }
        MILO_LOG("%s\n", str.c_str());
        str = "\tOffset Z";
        for (Difficulty d = EasiestDifficulty(); d < kNumDifficulties;
             d = DifficultyOneHarder(d)) {
            str += MakeString("\t%f", allDatas[d].mOffset.z);
        }
        MILO_LOG("%s\n", str.c_str());
    }
}

void SongCollision::Init() {
    REGISTER_OBJ_FACTORY(SongCollision);
    DataArray *tolerance = DataGetMacro("SONG_COLLISION_TOLERANCE");
    if (tolerance) {
        sCollisionTolerance = tolerance->Float(0);
    }
    sCollisionUsefulBoneNames.clear();
    DataArray *bones = DataGetMacro("SONG_COLLISION_BONES");
    if (bones) {
        for (int i = 0; i < bones->Size(); i++) {
            sCollisionUsefulBoneNames.push_back(bones->Str(i));
        }
    }
}

const BeatCollisionData *SongCollision::BeatData(int beat, Difficulty diff) const {
    MILO_ASSERT_RANGE(diff, 0, kNumDifficulties, 0xe8);
    const std::vector<BeatCollisionData> &diffData = mData[diff];
    MILO_ASSERT(beat >= 0, 0xeb);
    if (beat < diffData.size()) {
        return &diffData[beat];
    } else {
        return nullptr;
    }
}
