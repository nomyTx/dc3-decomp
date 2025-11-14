#include "synth/Sequence.h"
#include "math/Rand.h"
#include "obj/Object.h"

bool sForceSerialSequences;

namespace {
    float RandomVal(float f1, float f2) {
        if (f2 == 0.0f)
            return f1;
        else
            return RandomFloat(f1 - f2, f1 + f2);
    }
}

#pragma region Sequence

Sequence::Sequence()
    : mInsts(this), mAvgVol(0.0f), mVolSpread(0.0f), mAvgTranspose(0.0f),
      mTransposeSpread(0.0f), mAvgPan(0.0f), mPanSpread(0.0f), mFaders(this),
      mCanStop(true) {}

Sequence::~Sequence() {
    while (!mInsts.empty()) {
        delete mInsts.front();
    }
}

BEGIN_HANDLERS(Sequence)
    HANDLE(play, OnPlay)
    HANDLE_ACTION(stop, Stop(_msg->Size() == 4 ? _msg->Int(3) : 0))
    HANDLE_ACTION(add_fader, mFaders.Add(_msg->Obj<Fader>(2)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(Sequence)
    SYNC_PROP(avg_volume, mAvgVol)
    SYNC_PROP(volume_spread, mVolSpread)
    SYNC_PROP(avg_transpose, mAvgTranspose)
    SYNC_PROP(transpose_spread, mTransposeSpread)
    SYNC_PROP(avg_pan, mAvgPan)
    SYNC_PROP(pan_spread, mPanSpread)
    SYNC_PROP(can_stop, mCanStop)
    SYNC_PROP_SET(trigger_sound, 0, OnTriggerSound(_val.Int()))
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(Sequence)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mAvgVol;
    bs << mVolSpread;
    bs << mAvgTranspose;
    bs << mTransposeSpread;
    bs << mAvgPan;
    bs << mPanSpread;
    bs << mCanStop;
END_SAVES

BEGIN_COPYS(Sequence)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(Sequence)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mAvgVol)
            COPY_MEMBER(mVolSpread)
            COPY_MEMBER(mAvgTranspose)
            COPY_MEMBER(mTransposeSpread)
            COPY_MEMBER(mAvgPan)
            COPY_MEMBER(mPanSpread)
            COPY_MEMBER(mCanStop)
        }
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(Sequence)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    if (d.rev > 2) {
        Hmx::Object::Load(bs);
    }
    bs >> mAvgVol;
    bs >> mVolSpread;
    bs >> mAvgTranspose;
    bs >> mTransposeSpread;
    bs >> mAvgPan;
    bs >> mPanSpread;
    if (d.rev >= 2) {
        d >> mCanStop;
    }
END_LOADS

void Sequence::SynthPoll() {
    for (ObjPtrList<SeqInst>::iterator it = mInsts.begin(); it != mInsts.end(); it) {
        SeqInst *curSeq = *it++;
        curSeq->Poll();
        if (curSeq->Started() && !curSeq->IsRunning()) {
            delete curSeq;
        }
    }
    if (mFaders.Dirty()) {
        FOREACH (it, mInsts) {
            (*it)->UpdateVolume();
        }
    }
    if (mInsts.empty())
        CancelPolling();
}

SeqInst *Sequence::Play(float f1, float f2, float f3) {
    SeqInst *seq = MakeInst();
    if (seq) {
        seq->SetVolume(f1);
        seq->SetPan(f2);
        seq->SetTranspose(f3);
        seq->Start();
    }
    return seq;
}

void Sequence::Stop(bool b) {
    if (mCanStop || b) {
        std::for_each(mInsts.begin(), mInsts.end(), std::mem_fun(&SeqInst::Stop));
    }
}

SeqInst *Sequence::MakeInst() {
    SeqInst *seq = MakeInstImpl();
    if (seq) {
        mInsts.push_back(seq);
        StartPolling();
    }
    return seq;
}

void Sequence::OnTriggerSound(int i) {
    switch (i) {
    case 0: {
        Stop(false);
        break;
    }
    case 1: {
        Play(0.0f, 0.0f, 0.0f);
        break;
    }
    case 2: {
        ObjPtrList<SeqInst>::iterator it = mInsts.begin();
        for (; it != mInsts.end(); ++it) {
            if ((*it)->IsRunning())
                break;
        }
        if (it == mInsts.end())
            Play(0.0f, 0.0f, 0.0f);
        break;
    }
    default:
        break;
    }
}

DataNode Sequence::OnPlay(DataArray *arr) {
    static Symbol volume("volume");
    static Symbol pan("pan");
    static Symbol transpose("transpose");
    float fvol = 0.0f;
    float fpan = 0.0f;
    float ftrans = 0.0f;
    arr->FindData(volume, fvol, false);
    arr->FindData(pan, fpan, false);
    arr->FindData(transpose, ftrans, false);
    Play(fvol, fpan, ftrans);
    return 0;
}

void Sequence::Init() {
    SfxSeq::Init();
    WaitSeq::Init();
    RandomGroupSeq::Init();
    SerialGroupSeq::Init();
    ParallelGroupSeq::Init();
    RandomIntervalGroupSeq::Init();
}

#pragma endregion
#pragma region WaitSeq

WaitSeq::WaitSeq() : mAvgWaitSecs(0), mWaitSpread(0) {}

BEGIN_PROPSYNCS(WaitSeq)
    SYNC_PROP(avg_wait_seconds, mAvgWaitSecs)
    SYNC_PROP(wait_spread, mWaitSpread)
    SYNC_SUPERCLASS(Sequence)
END_PROPSYNCS

BEGIN_SAVES(WaitSeq)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Sequence)
    bs << mAvgWaitSecs;
    bs << mWaitSpread;
END_SAVES

BEGIN_COPYS(WaitSeq)
    COPY_SUPERCLASS(Sequence)
    CREATE_COPY(WaitSeq)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mAvgWaitSecs)
            COPY_MEMBER(mWaitSpread)
        }
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(WaitSeq)
    int rev;
    bs >> rev;
    if (rev > 2) {
        MILO_NOTIFY("Can't load new WaitSeq");
    } else {
        Sequence::Load(bs);
        bs >> mAvgWaitSecs;
        if (rev >= 2) {
            bs >> mWaitSpread;
        }
    }
END_LOADS

// SeqInst *WaitSeq::MakeInstImpl() { return new WaitSeqInst(this); }

#pragma endregion
#pragma region GroupSeq

GroupSeq::GroupSeq() : mChildren(this) {}

BEGIN_PROPSYNCS(GroupSeq)
    SYNC_PROP(children, mChildren)
    SYNC_SUPERCLASS(Sequence)
END_PROPSYNCS

BEGIN_SAVES(GroupSeq)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Sequence)
    bs << mChildren;
END_SAVES

BEGIN_COPYS(GroupSeq)
    COPY_SUPERCLASS(Sequence)
    CREATE_COPY(GroupSeq)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mChildren)
        }
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(GroupSeq)
    int rev;
    bs >> rev;
    if (rev > 3) {
        MILO_NOTIFY("Can't load new SfxSeq");
    } else {
        if (rev >= 2) {
            Sequence::Load(bs);
        }
        if (rev < 3) {
            mChildren.clear();
            ObjVector<ObjPtr<Sequence> > seqs(this);
            bs >> seqs;
            for (int i = 0; i < seqs.size(); i++) {
                mChildren.push_back(seqs[i]);
            }
        } else {
            bs >> mChildren;
        }
    }
END_LOADS

#pragma endregion
#pragma region RandomGroupSeq

RandomGroupSeq::RandomGroupSeq()
    : mNumSimul(1), mAllowRepeats(0), mNextIndex(-1), mForceChooseIndex(-1) {}

BEGIN_HANDLERS(RandomGroupSeq)
    HANDLE_EXPR(get_next_play_index, NextIndex())
    HANDLE_ACTION(force_next_play_index, ForceNextIndex(_msg->Int(2)))
    HANDLE_SUPERCLASS(Sequence)
END_HANDLERS

BEGIN_PROPSYNCS(RandomGroupSeq)
    SYNC_PROP(num_simul, mNumSimul)
    SYNC_PROP(allow_repeats, mAllowRepeats)
    SYNC_PROP(force_choose_index, mForceChooseIndex)
    SYNC_SUPERCLASS(GroupSeq)
END_PROPSYNCS

BEGIN_SAVES(RandomGroupSeq)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(GroupSeq)
    bs << mNumSimul;
    bs << mAllowRepeats;
END_SAVES

BEGIN_COPYS(RandomGroupSeq)
    COPY_SUPERCLASS(GroupSeq)
    CREATE_COPY(RandomGroupSeq)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mNumSimul)
            COPY_MEMBER(mAllowRepeats)
        }
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(RandomGroupSeq)
    int rev;
    bs >> rev;
    if (rev > 2) {
        MILO_NOTIFY("Can't load new RandomGroupSeq");
    } else {
        GroupSeq::Load(bs);
        bs >> mNumSimul;
        if (rev >= 2) {
            bs >> mAllowRepeats;
        }
    }
END_LOADS

void RandomGroupSeq::ForceSerialSequences(bool force) { sForceSerialSequences = force; }
bool RandomGroupSeq::UsingSerialSequences() { return sForceSerialSequences; }

void RandomGroupSeq::AddToPlayedHistory(int idx) {
    if (!mAllowRepeats) {
        if (mPlayHistory.size() != 0) {
            if (mPlayHistory.size() == mChildren.size() - 1) {
                int numChildren = mChildren.size() / 2;
                for (int i = 0; i < numChildren; i++) {
                    mPlayHistory.pop_front();
                }
            }
        }
        mPlayHistory.push_back(idx);
    }
}

int RandomGroupSeq::NextIndex() {
    if (mNextIndex == -1 && mChildren.size() != 0)
        PickNextIndex();
    return mNextIndex;
}

void RandomGroupSeq::ForceNextIndex(int i) {
    MILO_ASSERT(GetNumSimul() == 1 || Children().size() == 1, 0x1E0);
    if (i < 0 || i > Children().size() - 1) {
        MILO_NOTIFY("index out of bounds for ForceNextIndex (index = %d)", i);
    } else {
        mPlayHistory.remove(mNextIndex);
        mNextIndex = i;
    }
}

#pragma endregion
#pragma region RandomIntervalGroupSeq

RandomIntervalGroupSeq::RandomIntervalGroupSeq()
    : mAvgIntervalSecs(4), mIntervalSpread(2), mMaxSimultaneous(8) {}

BEGIN_PROPSYNCS(RandomIntervalGroupSeq)
    SYNC_PROP(max_simul, mMaxSimultaneous)
    SYNC_PROP(avg_interval_secs, mAvgIntervalSecs)
    SYNC_PROP(interval_spread, mIntervalSpread)
    SYNC_SUPERCLASS(GroupSeq)
END_PROPSYNCS

BEGIN_SAVES(RandomIntervalGroupSeq)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(GroupSeq)
    bs << mAvgIntervalSecs;
    bs << mIntervalSpread;
    bs << mMaxSimultaneous;
END_SAVES

BEGIN_COPYS(RandomIntervalGroupSeq)
    COPY_SUPERCLASS(GroupSeq)
    CREATE_COPY(RandomIntervalGroupSeq)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mAvgIntervalSecs)
            COPY_MEMBER(mIntervalSpread)
            COPY_MEMBER(mMaxSimultaneous)
        }
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(RandomIntervalGroupSeq)
    int rev;
    bs >> rev;
    if (rev > 1)
        MILO_NOTIFY("Can't load new RandomGroupSeq");
    else {
        GroupSeq::Load(bs);
        bs >> mAvgIntervalSecs >> mIntervalSpread >> mMaxSimultaneous;
    }
END_LOADS

#pragma endregion
#pragma region SerialGroupSeq

BEGIN_SAVES(SerialGroupSeq)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(GroupSeq)
END_SAVES

BEGIN_LOADS(SerialGroupSeq)
    int rev;
    bs >> rev;
    if (rev > 1) {
        MILO_NOTIFY("Can't load new SerialGroupSeq");
    } else {
        GroupSeq::Load(bs);
    }
END_LOADS

#pragma endregion
#pragma region ParallelGroupSeq

BEGIN_SAVES(ParallelGroupSeq)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(GroupSeq)
END_SAVES

BEGIN_LOADS(ParallelGroupSeq)
    int rev;
    bs >> rev;
    if (rev > 1) {
        MILO_NOTIFY("Can't load new ParallelGroupSeq");
    } else {
        GroupSeq::Load(bs);
    }
END_LOADS

#pragma endregion
#pragma region SfxSeq

SfxSeq::SfxSeq() {}

BEGIN_SAVES(SfxSeq)
    SAVE_REVS(4, 0)
    SAVE_SUPERCLASS(SerialGroupSeq)
END_SAVES

BEGIN_LOADS(SfxSeq)
    int rev;
    bs >> rev;
    if (rev > 4) {
        MILO_NOTIFY("Can't load new SfxSeq");
    } else if (rev <= 3) {
        if (rev <= 2) {
            Hmx::Object::Load(bs);
        } else {
            Sequence::Load(bs);
        }
        mChildren.clear();
        ObjPtr<Sequence> seq(this);
        bs >> seq;
        if (seq) {
            mChildren.push_back(seq);
        }
        if (rev == 2) {
            bs >> mAvgVol;
            bs >> mVolSpread;
            bs >> mAvgTranspose;
            bs >> mTransposeSpread;
            bs >> mAvgPan;
            bs >> mPanSpread;
        }
    } else {
        SerialGroupSeq::Load(bs);
    }
END_LOADS

#pragma endregion
#pragma region SeqInst

SeqInst::SeqInst(Sequence *seq) : mOwner(seq), mVolume(0.0f), mStarted(false) {
    mRandVol = RandomVal(mOwner->AvgVol(), mOwner->VolSpread());
    mRandPan = RandomVal(mOwner->AvgPan(), mOwner->PanSpread());
    mRandTp = RandomVal(mOwner->AvgTranspose(), mOwner->TransposeSpread());
}

void SeqInst::Start() {
    mStarted = true;
    StartImpl();
}

void SeqInst::SetVolume(float f) {
    mVolume = f;
    UpdateVolume();
}
