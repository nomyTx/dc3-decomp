#pragma once
#include "game/Game.h"
#include "gesture/FitnessFilter.h"
#include "obj/Data.h"
#include "os/Timer.h"
#include "rndobj/Overlay.h"
#include "ui/UIPanel.h"
#include "utl/Profiler.h"

class GamePanel : public UIPanel {
public:
    GamePanel();
    // Hmx::Object
    virtual ~GamePanel();
    OBJ_CLASSNAME(GamePanel);
    OBJ_SET_TYPE(GamePanel);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void SetTypeDef(DataArray *);
    // UIPanel
    virtual void Load();
    virtual void Enter();
    virtual void Exit();
    virtual void Poll();
    virtual void SetPaused(bool);
    virtual bool IsLoaded() const;
    virtual void Unload();
    virtual void PollForLoading();
    virtual void FinishLoad();

    void SetGameOver(bool);
    bool IsPastStreamJumpPointOfNoReturn();
    void ResetLimbFeedback();
    void SetLimbFeedbackVisible(bool);

    DataNode OnGetFitnessData(const DataArray *);

private:
    void StartGame();
    void SetPausedHelper(bool, bool);
    void CheatPause(bool);

    DataNode OnStartLoadSong(DataArray *);
    DataNode OnStartSongNow(DataArray *);
    DataNode OnMsg(const EndGameMsg &);

protected:
    void ClearDrawGlitch();
    void ReloadData();

    int unk38; // 0x38
    FitnessFilter mFitnessFilters[2]; // 0x3c
    RndOverlay *unk6c; // 0x6c
    RndOverlay *unk70; // 0x70
    RndOverlay *unk74; // 0x74
    RndOverlay *unk78; // 0x78
    bool unk7c;
    int unk80;
    int unk84;
    Profiler unk88;
    bool unkd8;
    std::vector<float> unkdc;
    int unke8;
    int unkec;
    float unkf0;
    int unkf4;
    bool unkf8;
    Timer *unkfc;
    bool unk100;
    bool unk101;
    int unk104;
    bool unk108;
};

extern GamePanel *TheGamePanel;
