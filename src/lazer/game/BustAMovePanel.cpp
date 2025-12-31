#include "BustAMovePanel.h"
#include "flow/PropertyEventProvider.h"
#include "game/GamePanel.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/FreestyleMoveRecorder.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamLabel.h"
#include "hamobj/HamMaster.h"
#include "hamobj/ScoreUtl.h"
#include "lazer/game/GameMode.h"
#include "lazer/meta_ham/HamPanel.h"
#include "math/Easing.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"
#include "rndobj/Dir.h"
#include "rndobj/Tex.h"
#include "rndobj/TexRenderer.h"
#include "ui/UIColor.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"
#include <float.h>

BustAMovePanel::BustAMovePanel()
    : unk40(0), unk44(0), unk58(-1), unk5c(0), unk60(0), unk64(0), unk68(4), unk6c(0),
      unk70(10), unk80(0), unka0(1), unk92c(0), unk930(0), unk934(0), unk958(-1),
      unk95c(-1), unk968(-1), unk970(0), unk988(0), unk98c(0), unk99c(0), unk9a0(FLT_MAX),
      unk9b9(0), unk9bc(-1) {
    unk40 = new FreestyleMoveRecorder();
    unk40->AssignStaticInstance();
}

BustAMovePanel::~BustAMovePanel() { delete unk40; }

BEGIN_HANDLERS(BustAMovePanel)
    HANDLE_ACTION(beat, OnBeat())
    HANDLE_ACTION(cache_objects, CacheObjects())
    HANDLE_ACTION(set_up_song_structure, SetUpSongStructure(_msg->Sym(2)))
    HANDLE_ACTION(on_stream_jump, unk98c = true)
    HANDLE_ACTION(play_intro_vo, PlayIntroVO())
    HANDLE_SUPERCLASS(HamPanel)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(BustAMovePanel)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void BustAMovePanel::Draw() {
    UIPanel::Draw();
    unk40->DrawDebug();
    if (unk934) {
        RndDir *renderer = DataDir()->Find<RndDir>("bustamove_flashcard_renderer");
        String flashcard(MakeString("flashcard%i.tex", unk84));
        RndTexRenderer *texRenderer =
            renderer->Find<RndTexRenderer>("TexRenderer.rndtex");
        int numPoses = 0;
        for (int i = 0; i < 3; i++) {
            if (unka4[i].Tracked()) {
                numPoses++;
                MILO_ASSERT(numPoses <= 2, 0x1BC);
                String pose(MakeString("pose%i.tex", numPoses));
                RndDir *renderer =
                    DataDir()->Find<RndDir>("bustamove_flashcard_renderer");
                RndTex *tex = renderer->Find<RndTex>(pose.c_str());
                TheHamDirector->PoseIconMan(&unka4[i], tex);
            }
        }
        if (numPoses == 0) {
            unk9b9 = true;
        }
        RndAnimatable *anim = renderer->Find<RndAnimatable>("num_poses.anim");
        anim->SetFrame(numPoses, 1);
        texRenderer->SetOutputTexture(DataDir()->Find<RndTex>(flashcard.c_str()));
        renderer->DrawShowing();
        unk934--;
    }
}

void BustAMovePanel::Enter() {
    HamPanel::Enter();
    unk60 = 0;
    if (InBustAMove()) {
        CacheObjects();
        unk9b8 = true;
    }
}

void BustAMovePanel::Exit() {
    UIPanel::Exit();
    TheMaster->RemoveSink(this);
    if (unk40) {
        unk40->Free();
    }
    TheHamDirector->SetPlayerSpotlightsEnabled(true);
}

bool BustAMovePanel::InBustAMove() {
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol bustamove("bustamove");
    return TheGameMode->Property(gameplay_mode)->Sym() == bustamove;
}

Symbol BustAMovePanel::GetPlayerColor(int i1) {
    static Symbol is_in_party_mode("is_in_party_mode");
    bool pink;
    if (TheHamProvider->Property(is_in_party_mode)->Int()) {
        if (TheGameData->Player(i1)->Side() == kSkeletonRight) {
            pink = true;
        } else {
            pink = false;
        }
    } else {
        pink = i1 == 0;
    }
    if (pink) {
        return "pink";
    } else {
        return "blue";
    }
    return 0;
}

MoveRating BustAMovePanel::GetMoveRating(float f1) {
    if (f1 > 0.85f) {
        return kMoveRatingSuperPerfect;
    } else if (f1 > 0.70f) {
        return kMoveRatingPerfect;
    } else if (f1 > 0.4f) {
        return kMoveRatingAwesome;
    } else {
        return kMoveRatingOk;
    }
}

void BustAMovePanel::SetFlashcardText(int i1, int i2, Symbol s3) {
    HamLabel *label = unk98[i1]->Find<HamLabel>(MakeString("flashcard_%d.lbl", i2));
    label->SetTextToken(s3);
    label = unk98[i1 == 0]->Find<HamLabel>(MakeString("flashcard_%d.lbl", i2));
    if (unk3c == 8 || unk3c == 7) {
        label->SetTextToken(s3);
    } else {
        label->SetTextToken(gNullStr);
    }
}

DataArray *BustAMovePanel::GetMoveNameData(int index) {
    static Symbol bustamove_move_names("bustamove_move_names");
    MILO_ASSERT(index >= 0 && index < MAX_FREESTYLE_MOVES, 0x656);
    int nameIndex = unk93c[index];
    MILO_ASSERT(nameIndex >= 0 && nameIndex < mShuffledMoveNames.size(), 0x658);
    DataArray *arr = TheGamePanel->Property(bustamove_move_names)->Array();
    return arr->Array(nameIndex);
}

void BustAMovePanel::SetMovePrompt() {
    Symbol sym = GetMoveNameData(unk84)->Sym(0);
    // unk78->SetTextToken(sym);
    UIColor *movePromptColor = DataDir()->Find<UIColor>("move_prompt.color");
    UIColor *playerColor =
        DataDir()->Find<UIColor>(MakeString("%s.color", GetPlayerColor(unk64)));
    Hmx::Color color = playerColor->GetColor();
    movePromptColor->SetColor(color);
}

void BustAMovePanel::IncreaseScore(int i1, int i2) {
    static Symbol score("score");
    int oldScore = TheGameData->Player(i1)->Provider()->Property(score)->Int();
    TheGameData->Player(i1)->Provider()->SetProperty(score, oldScore + i2);
}

void BustAMovePanel::ResetScores() {
    static Symbol score("score");
    TheGameData->Player(0)->Provider()->SetProperty(score, 0);
    TheGameData->Player(1)->Provider()->SetProperty(score, 0);
}

void BustAMovePanel::SetFlashcardName(int i1, int i2, int i3) {
    Symbol s(gNullStr);
    if (i3 >= 0) {
        s = GetMoveNameData(i3)->Sym(1);
    }
    HamLabel *label = unk98[i1]->Find<HamLabel>(MakeString("flashcard_name_%d.lbl", i2));
    label->SetTextToken(s);
    label = unk98[i1 == 0]->Find<HamLabel>(MakeString("flashcard_name_%d.lbl", i2));
    if (unk3c == 8 || unk3c == 7) {
        label->SetTextToken(s);
    } else {
        label->SetTextToken(gNullStr);
    }
}

void BustAMovePanel::CountIn(int i1) {
    int f4 = (int)(TheTaskMgr.Beat() + 0.5f) + i1;
    static Message countInMsg("count_in", 0, 0);
    countInMsg[0] = f4;
    countInMsg[1] = f4;
    Handle(countInMsg, true);
}

void BustAMovePanel::ShowMoveRating(MoveRating mr, int side) {
    const char *sideStr = side == 0 ? "left" : "right";
    RndDir *feedbackDir =
        DataDir()->Find<RndDir>(MakeString("bustamove_text_feedback_%s", sideStr));
    static Symbol move_perfect("move_perfect");
    static Symbol move_awesome("move_awesome");
    static Symbol move_ok("move_ok");
    static Symbol move_bad("move_bad");
    static Message moveFinishedMsg("move_finished", 0, 0);
    moveFinishedMsg[0] = side;
    if (mr == kMoveRatingSuperPerfect) {
        RndAnimatable *anim =
            feedbackDir->Find<RndAnimatable>("move_flawless_right.anim");
        anim->Animate(0, 0, 0, nullptr, kEaseLinear, 0, false);
        moveFinishedMsg[1] = move_perfect;
    } else if (mr == kMoveRatingPerfect) {
        RndAnimatable *anim = feedbackDir->Find<RndAnimatable>("move_nice_right.anim");
        anim->Animate(0, 0, 0, nullptr, kEaseLinear, 0, false);
        moveFinishedMsg[1] = move_awesome;
    } else if (mr == kMoveRatingAwesome) {
        RndAnimatable *anim = feedbackDir->Find<RndAnimatable>("move_okay_right.anim");
        anim->Animate(0, 0, 0, nullptr, kEaseLinear, 0, false);
        moveFinishedMsg[1] = move_ok;
    } else if (mr == kMoveRatingOk) {
        moveFinishedMsg[1] = move_bad;
    }
    TheHamProvider->Handle(moveFinishedMsg, false);
}

void BustAMovePanel::SetRoundFailure() {
    static Message resultMessage("set_bustamove_result", 0, 0, 0);
    resultMessage[0] = unk64 == 0;
    resultMessage[1] = 0;
    resultMessage[2] = 0;
    TheHamProvider->Handle(resultMessage, false);
}

void BustAMovePanel::PlayMovePromptVO() {
    PlayVO(GetMoveNameData(unk84)->Sym(unka0 + 2));
}

float BustAMovePanel::GetMovePromptVOLength() {
    float len = 0;
    Symbol sym = GetMoveNameData(unk84)->Sym(unka0 + 2);
    static Message voLengthMsg("get_seq_length", 0);
    voLengthMsg[0] = sym;
    // unk60->Handle
    return len;
}

void BustAMovePanel::ShowGetReadyCard(Symbol s, SkeletonSide side) {
    unk98[side]->Find<HamLabel>("get_ready.lbl")->SetTextToken(s);
    static Message getReadyMsg("bustamove_get_ready", 0);
    getReadyMsg[0] = side;
    TheHamProvider->Handle(getReadyMsg, false);
}
