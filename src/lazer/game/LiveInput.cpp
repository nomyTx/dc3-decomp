#include "game/LiveInput.h"
#include "hamobj/HamAudio.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Task.h"

LiveInput::LiveInput(HamAudio &audio) : mAudio(audio), unk8(0) { mTimer.Restart(); }

float LiveInput::GetSongToTaskMgrMs() const {
    return TheProfileMgr.GetSongToTaskMgrMs(kGame);
}

void LiveInput::SetTimeOffset() {
    float f1 = TheTaskMgr.Seconds(TaskMgr::kRealTime) * 1000.0f;
    f1 = f1 - mTimer.SplitMs();
    unk8 = f1 - TheProfileMgr.GetSongToTaskMgrMs(kGame);
}

void LiveInput::SetPostWaitJumpOffset(float f1) {
    mTimer.Restart();
    unk8 = f1 - mTimer.Ms();
}
