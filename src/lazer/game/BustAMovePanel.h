#pragma once
#include "gesture/BaseSkeleton.h"
#include "hamobj/BustAMoveData.h"
#include "hamobj/DancerSkeleton.h"
#include "hamobj/FreestyleMoveRecorder.h"
#include "hamobj/ScoreUtl.h"
#include "lazer/meta_ham/HamPanel.h"
#include "obj/Dir.h"

class BustAMovePanel : public HamPanel {
public:
    BustAMovePanel();
    // Hmx::Object
    virtual ~BustAMovePanel();
    OBJ_CLASSNAME(BustAMovePanel);
    OBJ_SET_TYPE(BustAMovePanel);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    // UIPanel
    virtual void Draw();
    virtual void Enter();
    virtual void Exit();
    virtual void Poll();

    void OnBeat();
    void SetUpSongStructure(Symbol);
    void PlayVO(Symbol);

private:
    bool InBustAMove();

    void CacheObjects();
    float GetMovePromptVOLength();
    void PlayIntroVO();
    void QueueMovePromptVO();
    void PlayMovePromptVO();
    DataArray *GetMoveNameData(int);
    Symbol GetPlayerColor(int);
    MoveRating GetMoveRating(float);
    void SetFlashcardText(int, int, Symbol);
    void SetMovePrompt();
    void IncreaseScore(int, int);
    void ResetScores();
    void SetFlashcardName(int, int, int);
    void CountIn(int);
    void ShowMoveRating(MoveRating, int);
    void SetRoundFailure();
    void ShowGetReadyCard(Symbol, SkeletonSide);

    int unk3c; // 0x3c - BAMState
    FreestyleMoveRecorder *unk40;
    int unk44;
    std::list<Symbol> unk48;
    std::list<int> unk50;
    int unk58;
    float unk5c;
    int unk60;
    int unk64;
    int unk68;
    int unk6c;
    int unk70;
    int unk74;
    int unk78;
    int unk7c;
    float unk80;
    int unk84;
    int unk88;
    int unk8c;
    int unk90;
    int unk94;
    ObjectDir *unk98[2]; // 0x98
    int unka0;
    DancerSkeleton unka4[3]; // 0xa4
    int unk92c;
    float unk930;
    int unk934;
    int unk938;
    int unk93c[4];
    int unk94c;
    int unk950;
    int unk954;
    float unk958;
    float unk95c;
    int unk960;
    int unk964;
    int unk968;
    int unk96c;
    bool unk970;
    int unk974;
    int unk978;
    std::vector<int> unk97c;
    int unk988;
    bool unk98c;
    std::vector<int> mShuffledMoveNames; // 0x990
    int unk99c;
    float unk9a0;
    int unk9a4;
    int unk9a8;
    int unk9ac;
    int unk9b0;
    int unk9b4;
    bool unk9b8;
    bool unk9b9;
    int unk9bc;

    // u32 unk_0x3C;
    // u32 unk_0x40; // FreestyleMoveRecorder*
    // u32 unk_0x44;
    // double unk_0x48;
    // u32 unk_0x50;
    // u32 unk_0x54;
    // u32 unk_0x58; // FreestyleMoveRecorder*
    // u8 pad[0x24];
    // u32 unk_0x84;
    // u8 pad2[0xc];

    // ObjectDir *mDirs[2]; // 0x94
    // u32 unk_0x9C;
    // u32 unk_0xA0;
    // DancerSkeleton mSkeletons[3]; // 0xA4

    // bool unk_0x9B8;
};
