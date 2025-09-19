#include "hamobj/CharFeedback.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"

bool CharFeedback::sEnabled = true;

CharFeedback::CharFeedback()
    : mTarget(this), mFailMat(this), mFailTriggerSecs(0.1), mMinFailSecs(0.2),
      mFadeSecs(1), mTestLimbs(kFeedbackNone) {
    Sync();
}

CharFeedback::~CharFeedback() {}

BEGIN_HANDLERS(CharFeedback)
    HANDLE_ACTION(trigger_fail, TestUpdateLimbs(_msg->Int(2)))
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharFeedback)
    SYNC_PROP_MODIFY(target, mTarget, Sync())
    SYNC_PROP_MODIFY(fail_mat, mFailMat, Sync())
    SYNC_PROP(fail_trigger_secs, mFailTriggerSecs)
    SYNC_PROP(min_fail_secs, mMinFailSecs)
    SYNC_PROP_SET(test_limbs, (int &)mTestLimbs, mTestLimbs = (FeedbackLimbs)_val.Int())
    SYNC_PROP(fade_secs, mFadeSecs)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharFeedback)
    SAVE_REVS(10, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mTarget;
    bs << mTestLimbs;
    bs << mFailTriggerSecs;
    bs << mMinFailSecs;
    bs << mFailMat;
    bs << mFadeSecs;
END_SAVES

BEGIN_COPYS(CharFeedback)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(CharFeedback)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mTarget)
        COPY_MEMBER(mTestLimbs)
        COPY_MEMBER(mFailTriggerSecs)
        COPY_MEMBER(mMinFailSecs)
        COPY_MEMBER(mFailMat)
        COPY_MEMBER(mFadeSecs)
    END_COPYING_MEMBERS
END_COPYS

void CharFeedback::Enter() {
    RndPollable::Enter();
    Sync();
}

void CharFeedback::TestUpdateLimbs(bool b1) {
    Sync();
    for (int i = 0; i < kNumLimbFeedbacks; i++) {
        if (mTestLimbs & (1 << i)) {
            UpdateLimb(i, b1);
        }
    }
}
