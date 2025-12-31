#pragma once
#include "hamobj/Difficulty.h"
#include "math/DoubleExponentialSmoother.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/Playlist.h"
#include "net_ham/PartyModeJobs.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/DateTime.h"
#include "utl/MemMgr.h"
#include "utl/PseudoRandomPicker.h"

DECLARE_MESSAGE(SmartGlassMsg, "smart_glass_msg")
END_MESSAGE

class PartyModeARObject {
public:
    PartyModeARObject(DataArray *a) : unk8(a) {}
    const char *GetTexPath();

    MEM_OVERLOAD(PartyModeARObject, 0x4C);

    int unk0;
    int unk4;
    DataArray *unk8;
};

class PartyModePlayer {
public:
    PartyModePlayer(PartyModeARObject *);
    ~PartyModePlayer();

    void PushTitle(Symbol);
    int Index() const { return unk1c; }
    void IncScore(int score) { unk14 += score; }
    void StoreFramePos(float x, float y) {
        unk20 = x;
        unk24 = y;
    }
    void StoreFrameScale(float scale) { unk28 = scale; }
    const char *GetTexPath() { return unk0->GetTexPath(); }
    int GetPhotoIndex() const { return unk2c; }
    void SetIndex(int idx) { unk1c = idx; }
    void SetSym(Symbol s) { unk4 = s; } // rename once context known
    void SetPhotoIndex(int idx) { unk2c = idx; }

private:
    PartyModeARObject *unk0;
    Symbol unk4;
    std::list<Symbol> unk8;
    DataArray *unk10;
    int unk14;
    int unk18;
    int unk1c;
    float unk20;
    float unk24;
    float unk28;
    int unk2c;
};

class PartyModeMgr : public Hmx::Object, public ContentMgr::Callback {
public:
    struct ConfigHistory {
        int unk0;
        int unk4;
        int unk8;
        int unkc;
    };
    struct SubMode {
        ~SubMode();

        MEM_OVERLOAD(SubMode, 0xE3);

        Symbol mName; // 0x0
        Symbol mMicrogameName; // 0x4
        int unk8;
        int mSongID; // 0xc
        int mPlayerFlags; // 0x10
        int mNumPlayers; // 0x14
        int unk18;
        int unk1c;
        int unk20;
        int unk24;
        DataArray *mPlayers; // 0x28
    };
    PartyModeMgr();
    // Hmx::Object
    virtual ~PartyModeMgr();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    // ContentMgr::Callback
    virtual void ContentMounted(const char *contentName, const char *);

    static void Init();

    void ResetPlayers();
    void AddPlayerToTeam(int);
    void FinalizeTeam(int);
    void ClearTeam(int);
    void FinalizeParty();
    void StorePlayerFramePos(int, float, float);
    void StorePlayerFrameScale(int, float);
    const char *GetPlayerARTexPath(int);
    void ResetParty();
    void CrewShowdownRematch();
    int GetPlayerPhotoIndex(int);
    int GetLeftPlayerIndex() const;
    int GetRightPlayerIndex() const;
    void IncLeftPlayerScore(int);
    void IncRightPlayerScore(int);
    void PushLeftPlayerTitle(Symbol);
    void PushRightPlayerTitle(Symbol);
    void SetLeftTeamScore(float);
    void SetRightTeamScore(float);
    void IncLeftTeamScore(float);
    void IncRightTeamScore(float);
    void StartNewRound();
    float GetPointsForWin();
    float GetPointsForLoss();
    void ToggleIncludedModeOn(Symbol, bool);
    bool IsModeIncluded(Symbol);
    DataArray *GetLeftCrewColor1AsArray();
    DataArray *GetLeftCrewColor2AsArray();
    DataArray *GetRightCrewColor1AsArray();
    DataArray *GetRightCrewColor2AsArray();
    int GetCrewColor(int, int);
    bool LeftTeamMaxWins() const;
    bool RightTeamMaxWins() const;
    void SetLeftTeamStarBonus();
    void SetRightTeamStarBonus();

private:
    void InitCharacters();
    Symbol GetCurrEventName();
    Symbol GetCurrEventMicrogameName();
    Symbol GetCurrEventSongName();
    Symbol GetCurrEventSongShortName();
    int GetCurrEventPlayerFlags();
    int GetCurrEventNumPlayers();
    Symbol GetCurrEventSongArtistName();
    DataArray *GetCurrEventPlayers();
    void SetCurrEvent();
    void UpdateRoundsPlayed();
    void SetRandomCharacters();
    void SetupCharacterData();
    bool IsTeamSignedIn(int);
    void SmoothFrameMotion(int, float, float);
    void ForceFrameSmootherPos(int, float, float);
    void UpdateScores();
    void UseSelectedPlaylist(bool);
    void ShufflePlaylist(bool);
    void ToggleIncludedMode(Symbol);
    void SetModes();
    void SetupInfinitePartyMode();
    const char *GetPlaylistString();
    Symbol GetLeftCrewCharOutfit(int, int);
    Symbol GetRightCrewCharOutfit(int, int);
    void SendPartyOptionsToRC();
    void GetPartyOptionsFromRC();
    void GetPartySongQueueFromRC();
    Symbol GetNextSongName();
    void ChangeToAnotherGameMode();
    void EndPartyStats();
    void OnSmartGlassListen(int);
    void PruneHistory();
    HamProfile *GetValidProfile();
    void BroadcastSyncMsg(Symbol);
    void ReadPartyOptions();
    void DeleteSongFromRCPartySongQueue(int);
    void AddNextSongToRCPartySongQueue();
    Symbol GetNextMode();
    void DetermineSubMode(Symbol *, Symbol *);
    void DetermineSubModeSong(Symbol *, int *);
    PartyModePlayer *CreatePartyModePlayer();
    void ResetMicrogames();
    int PickNextPlayer();
    void SetSongsFromPlaylist();

    Symbol GetCurrEventDisplayName() { return GetCurrEventName(); }
    bool IsUsingPlaylist() const { return unk2d4 != 0; }

    DataNode OnGetSmoothedFramePos(const DataArray *);
    DataNode OnSetSongAndDefaults(DataArray *);
    DataNode OnStableSong();
    DataNode OnStableMode();
    DataNode OnMsg(const RCJobCompleteMsg &);
    DataNode OnMsg(const SmartGlassMsg &);

    DataArray *mPartyModeCfg; // 0x30
    bool mUsePlaytestData; // 0x34
    DataArray *mPartyModePlaytestData; // 0x38
    DataArray *mPartyModePlaytestEvents; // 0x3c
    bool unk40;
    SubMode *mCurrEvent; // 0x44
    int unk48;
    int unk4c;
    int unk50;
    bool unk54;
    DataArray *mARObjects; // 0x58
    std::vector<PartyModePlayer *> mPlayers; // 0x5c
    std::vector<PartyModePlayer *> unk68;
    std::vector<PartyModePlayer *> unk74;
    float unk80;
    float unk84;
    float unk88;
    float unk8c;
    float unk90;
    float unk94;
    DataArray *mEventScoring; // 0x98
    int unk9c;
    int unka0;
    std::vector<int> unka4;
    std::vector<int> unkb0;
    PseudoRandomPicker<int> unkbc;
    PseudoRandomPicker<int> unkd0;
    PseudoRandomPicker<int> unke4;
    PseudoRandomPicker<Symbol> unkf8;
    PseudoRandomPicker<Symbol> mModePicker; // 0x10c
    PseudoRandomPicker<Symbol> unk120;
    PseudoRandomPicker<Symbol> unk134; // 0x134 - good items
    PseudoRandomPicker<Symbol> unk148; // 0x148 - bad items
    PseudoRandomPicker<Symbol> unk15c[4];
    std::vector<Symbol> unk1ac;
    Symbol unk1b8;
    Symbol unk1bc;
    PartyModePlayer *unk1c0; // 0x1c0 - left player
    PartyModePlayer *unk1c4; // 0x1c4 - right player
    int unk1c8;
    DataArray *mGoodTitles; // 0x1cc
    DataArray *mBadTitles; // 0x1d0
    std::vector<int> unk1d4;
    Vector2DESmoother unk1e0[6];
    Difficulty unk2d0;
    Playlist *unk2d4;
    bool unk2d8;
    bool mUseFullLengthSongs; // 0x2d9
    int unk2dc;
    bool unk2e0;
    bool unk2e1;
    bool unk2e2;
    float unk2e4;
    float unk2e8;
    float unk2ec;
    int unk2f0;
    SetPartyOptionsJob *unk2f4;
    GetPartyOptionsJob *unk2f8;
    GetPartySongQueueJob *unk2fc;
    AddSongToPartySongQueueJob *unk300;
    DeleteSongFromPartySongQueueJob *unk304;
    std::list<SongQueueRow> unk308;
    int unk310;
    bool unk314;
    DateTime unk315;
    DateTime unk31b;
    DataArray *unk324;
    DataArray *unk328;
    DataArray *unk32c;
    std::vector<ConfigHistory> unk330;
};

extern PartyModeMgr *ThePartyModeMgr;
