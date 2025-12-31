#include "game/PartyModeMgr.h"
#include "flow/PropertyEventProvider.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamSongMetadata.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/SongRecord.h"
#include "meta_ham/Utl.h"
#include "net_ham/PartyModeJobs.h"
#include "net_ham/RCJobDingo.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/DataUtl.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/ContentMgr.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "ui/UI.h"
#include "utl/Locale.h"
#include "utl/Symbol.h"
#include <cstdlib>

PartyModeMgr *ThePartyModeMgr;

namespace {
    int GetEnumFromModeName(Symbol mode) {
        static Symbol bustamove("bustamove");
        static Symbol perform("perform");
        static Symbol dance_battle("dance_battle");
        static Symbol rhythm_battle("rhythm_battle");
        static Symbol strike_a_pose("strike_a_pose");
        if (mode == bustamove) {
            return 3;
        } else if (mode == perform) {
            return 0;
        } else if (mode == dance_battle) {
            return 1;
        } else if (mode == rhythm_battle) {
            return 2;
        } else if (mode == strike_a_pose) {
            return 4;
        } else {
            return 0x20;
        }
    }

    Symbol GetModeNameFromEnum(int enumType) {
        static Symbol bustamove("bustamove");
        static Symbol perform("perform");
        static Symbol dance_battle("dance_battle");
        static Symbol rhythm_battle("rhythm_battle");
        static Symbol strike_a_pose("strike_a_pose");
        switch (enumType) {
        case 0:
            return perform;
        case 1:
            return dance_battle;
        case 2:
            return rhythm_battle;
        case 3:
            return bustamove;
        case 4:
            return strike_a_pose;
        default:
            MILO_ASSERT(0, 0x49);
            return gNullStr;
        }
    }
}

#pragma region PartyModeARObject

const char *PartyModeARObject::GetTexPath() {
    const char *texPath = gNullStr;
    static Symbol image_path("image_path");
    DataArray *pathArr = unk8->FindArray(image_path);
    if (pathArr) {
        texPath = pathArr->Str(1);
    }
    return texPath;
}

#pragma endregion
#pragma region PartyModePlayer

PartyModePlayer::PartyModePlayer(PartyModeARObject *obj) : unk0(obj), unk14(0) {
    unk10 = new DataArray(3);
}

PartyModePlayer::~PartyModePlayer() {
    RELEASE(unk0);
    unk10->Release();
}

void PartyModePlayer::PushTitle(Symbol s) {
    unk8.push_back(s);
    if (unk8.size() > 3) {
        unk8.pop_front();
    }
    int idx = 0;
    for (auto it = unk8.begin(); it != unk8.end(); ++it, ++idx) {
        unk10->Node(idx) = *it;
    }
}

#pragma endregion
#pragma region PartyModeMgr

PartyModeMgr::PartyModeMgr() : unk1e0() {
    unk40 = false;
    unk328 = 0;
    static Symbol party_mode("party_mode");
    mPartyModeCfg = SystemConfig(party_mode);
    static Symbol ar_objects("ar_objects");
    mARObjects = mPartyModeCfg->FindArray(ar_objects);
    static Symbol good_titles("good_titles");
    mGoodTitles = mPartyModeCfg->FindArray(good_titles);
    static Symbol bad_titles("bad_titles");
    mBadTitles = mPartyModeCfg->FindArray(bad_titles);
    static Symbol event_scoring("event_scoring");
    mEventScoring = mPartyModeCfg->FindArray(event_scoring);
    mUsePlaytestData = false;
    mPartyModePlaytestEvents = nullptr;
    static Symbol party_mode_playtest_data("party_mode_playtest_data");
    mPartyModePlaytestData = mPartyModeCfg->FindArray(party_mode_playtest_data);
    if (mPartyModePlaytestData) {
        static Symbol use_playtest_data("use_playtest_data");
        DataArray *useData = mPartyModePlaytestData->FindArray(use_playtest_data);
        if (useData && useData->Int(1) != 0) {
            mUsePlaytestData = true;
        }
    }
    if (mUsePlaytestData) {
        static Symbol party_mode_playtest_events("party_mode_playtest_events");
        mPartyModePlaytestEvents =
            mPartyModePlaytestData->FindArray(party_mode_playtest_events);
    }
    std::vector<Symbol> vec;
    int numGoodTitles = mGoodTitles->Size();
    vec.resize(numGoodTitles);
    for (int i = 1; i < numGoodTitles; i++) {
        vec[i - 1] = mGoodTitles->Sym(i);
    }
    unk134.AddItems(vec);
    // unk134.unk8 = 0;
    int numBadTitles = mBadTitles->Size();
    vec.resize(numBadTitles);
    for (int i = 1; i < numBadTitles; i++) {
        vec[i - 1] = mBadTitles->Sym(i);
    }
    unk148.AddItems(vec);
    // unk148.unk8 = 0;
    for (int i = 1; i <= mARObjects->Size() - 1; i++) {
        unkb0.push_back(i);
    }
    // a for loop with trapwords occurs here
    mCurrEvent = nullptr;
    InitCharacters();
    for (int i = 0; i < 6; i++) {
        unk1e0[i].SetSmoothParameters(10, 1);
        unk1e0[i].ForceValue(Vector2(0.5, 0.5));
    }
    unk2d0 = DefaultDifficulty();
    unk2d4 = 0;
    unk2d8 = false;
    unk2dc = -1;
    mUseFullLengthSongs = false;
    static DataNode &n = DataVariable("force_song_shortening_off");
    if (n.Int()) {
        mUseFullLengthSongs = true;
    }
    unk2e0 = false;
    unk2e1 = false;
    unk2e2 = false;
    unk2f4 = 0;
    unk2f8 = 0;
    unk2fc = 0;
    unk300 = 0;
    unk304 = 0;
    unk314 = false;
    unk324 = 0;
}

PartyModeMgr::~PartyModeMgr() { ResetPlayers(); }

BEGIN_HANDLERS(PartyModeMgr)
    HANDLE_ACTION(add_player_to_team, AddPlayerToTeam(_msg->Int(2)))
    HANDLE_ACTION(finalize_team, FinalizeTeam(_msg->Int(2)))
    HANDLE_ACTION(clear_team, ClearTeam(_msg->Int(2)))
    HANDLE_ACTION(finalize_party, FinalizeParty())
    HANDLE_ACTION(
        store_player_frame_pos,
        StorePlayerFramePos(_msg->Int(2), _msg->Float(3), _msg->Float(4))
    )
    HANDLE_ACTION(
        store_player_frame_scale, StorePlayerFrameScale(_msg->Int(2), _msg->Float(3))
    )
    HANDLE_EXPR(get_tex_path, GetPlayerARTexPath(_msg->Int(2)))
    HANDLE_EXPR(num_enrolled, (int)mPlayers.size())
    HANDLE_EXPR(num_enrolled_team_1, (int)unk68.size())
    HANDLE_EXPR(num_enrolled_team_2, (int)unk74.size())
    HANDLE_EXPR(get_curr_event_name, GetCurrEventName())
    HANDLE_EXPR(get_curr_event_display_name, GetCurrEventDisplayName())
    HANDLE_EXPR(get_curr_event_microgame_name, GetCurrEventMicrogameName())
    HANDLE_EXPR(get_curr_event_song_name, GetCurrEventSongName())
    HANDLE_EXPR(get_curr_event_song_shortname, GetCurrEventSongShortName())
    HANDLE_EXPR(get_curr_event_player_flags, GetCurrEventPlayerFlags())
    HANDLE_EXPR(get_curr_event_num_players, GetCurrEventNumPlayers())
    HANDLE_EXPR(get_curr_event_artist_name, GetCurrEventSongArtistName())
    HANDLE_EXPR(get_curr_event_players, GetCurrEventPlayers())
    HANDLE_ACTION(update_curr_event, SetCurrEvent())
    HANDLE_ACTION(update_rounds_played, UpdateRoundsPlayed())
    HANDLE_EXPR(get_max_participants, 8)
    HANDLE_ACTION(set_random_characters, SetRandomCharacters())
    HANDLE_ACTION(setup_character_data, SetupCharacterData())
    HANDLE_ACTION(reset_party, ResetParty())
    HANDLE_ACTION(crew_showdown_rematch, CrewShowdownRematch())
    HANDLE_EXPR(get_left_player_index, GetLeftPlayerIndex())
    HANDLE_EXPR(get_right_player_index, GetRightPlayerIndex())
    HANDLE_ACTION(inc_left_player_score, IncLeftPlayerScore(_msg->Int(2)))
    HANDLE_ACTION(inc_right_player_score, IncRightPlayerScore(_msg->Int(2)))
    HANDLE_EXPR(get_player_photo_index, GetPlayerPhotoIndex(_msg->Int(2)))
    HANDLE_ACTION(push_left_player_title, PushLeftPlayerTitle(_msg->Sym(2)))
    HANDLE_ACTION(push_right_player_title, PushRightPlayerTitle(_msg->Sym(2)))
    HANDLE_EXPR(is_showdown, unk54)
    HANDLE_EXPR(is_team_signed_in, IsTeamSignedIn(_msg->Int(2)))
    HANDLE_ACTION(set_left_team_score, SetLeftTeamScore(_msg->Float(2)))
    HANDLE_ACTION(set_right_team_score, SetRightTeamScore(_msg->Float(2)))
    HANDLE_ACTION(inc_left_team_score, IncLeftTeamScore(_msg->Float(2)))
    HANDLE_ACTION(inc_right_team_score, IncRightTeamScore(_msg->Float(2)))
    HANDLE_EXPR(get_left_team_score, unk84)
    HANDLE_EXPR(get_right_team_score, unk8c)
    HANDLE_EXPR(get_left_team_prev_score, unk80)
    HANDLE_EXPR(get_right_team_prev_score, unk88)
    HANDLE_ACTION(start_new_round, StartNewRound())
    HANDLE_ACTION(
        smooth_frame_motion,
        SmoothFrameMotion(_msg->Int(2), _msg->Float(3), _msg->Float(4))
    )
    HANDLE_ACTION(
        force_frame_smoother_pos,
        ForceFrameSmootherPos(_msg->Int(2), _msg->Float(3), _msg->Float(4))
    )
    HANDLE(get_smoothed_frame_pos, OnGetSmoothedFramePos)
    HANDLE_ACTION(set_difficulty, unk2d0 = (Difficulty)_msg->Int(2))
    HANDLE_EXPR(get_difficulty, unk2d0)
    HANDLE_ACTION(set_left_team_crew, unk1b8 = _msg->Sym(2))
    HANDLE_ACTION(set_right_team_crew, unk1bc = _msg->Sym(2))
    HANDLE_EXPR(get_left_team_crew, unk1b8)
    HANDLE_EXPR(get_right_team_crew, unk1bc)
    HANDLE_EXPR(get_points_for_win, GetPointsForWin())
    HANDLE_EXPR(get_points_for_loss, GetPointsForLoss())
    HANDLE_ACTION(update_scores, UpdateScores())
    HANDLE_ACTION(use_selected_playlist, UseSelectedPlaylist(_msg->Int(2)))
    HANDLE_EXPR(is_using_playlist, IsUsingPlaylist())
    HANDLE_ACTION(shuffle_playlist, ShufflePlaylist(_msg->Int(2)))
    HANDLE_EXPR(is_playlist_shuffled, unk2d8)
    HANDLE_ACTION(use_full_length_songs, mUseFullLengthSongs = _msg->Int(2))
    HANDLE_EXPR(is_using_full_length_songs, mUseFullLengthSongs)
    HANDLE_ACTION(toggle_included_mode, ToggleIncludedMode(_msg->Sym(2)))
    HANDLE_ACTION(
        toggle_included_mode_on, ToggleIncludedModeOn(_msg->Sym(2), _msg->Int(3))
    )
    HANDLE_ACTION(set_modes, SetModes())
    HANDLE_EXPR(is_mode_included, IsModeIncluded(_msg->Sym(2)))
    HANDLE_ACTION(setup_infinite_party_mode, SetupInfinitePartyMode())
    HANDLE(set_song_and_defaults, OnSetSongAndDefaults)
    HANDLE_EXPR(get_playlist_string, GetPlaylistString())
    HANDLE_ACTION(set_per_song_difficulty, unk2e0 = _msg->Int(2))
    HANDLE_EXPR(use_per_song_difficulty, unk2e0)
    HANDLE_ACTION(set_custom_party, unk2e1 = _msg->Int(2))
    HANDLE_EXPR(is_custom_party, unk2e1)
    HANDLE_EXPR(get_left_crew_color_1, GetLeftCrewColor1AsArray())
    HANDLE_EXPR(get_left_crew_color_2, GetLeftCrewColor2AsArray())
    HANDLE_EXPR(get_right_crew_color_1, GetRightCrewColor1AsArray())
    HANDLE_EXPR(get_right_crew_color_2, GetRightCrewColor2AsArray())
    HANDLE_EXPR(get_crew_color, GetCrewColor(_msg->Int(2), _msg->Int(3)))
    HANDLE_EXPR(
        get_left_crew_char_outfit, GetLeftCrewCharOutfit(_msg->Int(2), _msg->Int(3))
    )
    HANDLE_EXPR(
        get_right_crew_char_outfit, GetRightCrewCharOutfit(_msg->Int(2), _msg->Int(3))
    )
    HANDLE_EXPR(get_left_team_prev_pct_of_max_points, unk2e4)
    HANDLE_EXPR(get_right_team_prev_pct_of_max_points, unk2e8)
    HANDLE_EXPR(get_left_team_curr_pct_of_max_points, unk2e4 = unk84 / unk2ec)
    HANDLE_EXPR(get_right_team_curr_pct_of_max_points, unk2e8 = unk8c / unk2ec)
    HANDLE_EXPR(get_winning_side, unk9c)
    HANDLE_EXPR(get_just_won_side, unka0)
    HANDLE_EXPR(left_team_max_wins, LeftTeamMaxWins())
    HANDLE_EXPR(right_team_max_wins, RightTeamMaxWins())
    HANDLE_ACTION(send_party_options_to_rc, SendPartyOptionsToRC())
    HANDLE_ACTION(get_party_options_from_rc, GetPartyOptionsFromRC())
    HANDLE_ACTION(get_party_song_queue_from_rc, GetPartySongQueueFromRC())
    HANDLE_EXPR(get_next_song, GetNextSongName())
    HANDLE_ACTION(change_to_another_game_mode, ChangeToAnotherGameMode())
    HANDLE_EXPR(get_rounds_played, unk48)
    HANDLE_EXPR(get_rounds_total, unk4c)
    HANDLE_ACTION(start_party_stats, GetDateAndTime(unk31b))
    HANDLE_ACTION(end_party_stats, EndPartyStats())
    HANDLE_ACTION(smart_glass_listen, OnSmartGlassListen(_msg->Int(2)))
    HANDLE_ACTION(prune_history, PruneHistory())
    HANDLE_EXPR(stable_song, OnStableSong())
    HANDLE_EXPR(stable_mode, OnStableMode())
    HANDLE_MESSAGE(RCJobCompleteMsg)
    HANDLE_MESSAGE(SmartGlassMsg)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(PartyModeMgr)
    SYNC_PROP(is_playlist_shuffled, unk2d8)
    SYNC_PROP(is_using_per_song_options, unk2e2)
    SYNC_PROP(curr_synced_song_id, unk310)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void PartyModeMgr::ContentMounted(const char *contentName, const char *) {
    MILO_ASSERT(contentName, 0x154);
    if (!TheContentMgr.RefreshInProgress() && mCurrEvent) {
        if (TheHamSongMgr.IsContentUsedForSong(contentName, mCurrEvent->mSongID)) {
            static Symbol song_data_mounted("song_data_mounted");
            static Message msg(song_data_mounted, gNullStr);
            msg[0] = GetCurrEventSongShortName();
            TheUI->Export(msg, false);
        }
    }
}

void PartyModeMgr::Init() {
    MILO_ASSERT(ThePartyModeMgr == NULL, 0x142);
    ThePartyModeMgr = new PartyModeMgr();
    if (ObjectDir::Main()) {
        ThePartyModeMgr->SetName("partymode_mgr", ObjectDir::Main());
    }
    TheContentMgr.RegisterCallback(ThePartyModeMgr, false);
}

int PartyModeMgr::GetLeftPlayerIndex() const {
    int idx = -1;
    if (unk1c0) {
        idx = unk1c0->Index();
    }
    return idx;
}

int PartyModeMgr::GetRightPlayerIndex() const {
    int idx = -1;
    if (unk1c4) {
        idx = unk1c4->Index();
    }
    return idx;
}

void PartyModeMgr::IncLeftPlayerScore(int score) {
    if (unk1c0) {
        unk1c0->IncScore(score);
    }
}

void PartyModeMgr::IncRightPlayerScore(int score) {
    if (unk1c4) {
        unk1c4->IncScore(score);
    }
}

void PartyModeMgr::PushLeftPlayerTitle(Symbol title) {
    if (unk1c0) {
        unk1c0->PushTitle(title);
    }
}

void PartyModeMgr::PushRightPlayerTitle(Symbol title) {
    if (unk1c4) {
        unk1c4->PushTitle(title);
    }
}

void PartyModeMgr::SetLeftTeamScore(float score) {
    unk80 = unk84;
    unk84 = score;
}

void PartyModeMgr::SetRightTeamScore(float score) {
    unk88 = unk8c;
    unk8c = score;
}

void PartyModeMgr::IncLeftTeamScore(float score) {
    unk80 = unk84;
    unk84 += score;
}

void PartyModeMgr::IncRightTeamScore(float score) {
    unk88 = unk8c;
    unk8c += score;
}

void PartyModeMgr::StartNewRound() {
    unk80 = unk84;
    unk88 = unk8c;
    unk2e4 = 0;
    unk2e8 = 0;
    unk84 = 0;
    unk8c = 0;
}

bool PartyModeMgr::LeftTeamMaxWins() const {
    return 0.001f >= unk2ec - unk84 && unk9c == 0;
}

bool PartyModeMgr::RightTeamMaxWins() const {
    return 0.001f >= unk2ec - unk8c && unk9c == 1;
}

bool PartyModeMgr::IsModeIncluded(Symbol mode) {
    return (1 << GetEnumFromModeName(mode)) & unk2dc;
}

Symbol PartyModeMgr::GetNextSongName() {
    if (unk310 == 0) {
        return gNullStr;
    } else {
        return TheHamSongMgr.GetShortNameFromSongID(unk310, false);
    }
}

HamProfile *PartyModeMgr::GetValidProfile() {
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile) {
        profile->UpdateOnlineID();
        if (profile->IsSignedIn() && ThePlatformMgr.IsSignedIntoLive(profile->GetPadNum())
            && TheRockCentral.IsOnline()) {
            return profile;
        }
    }
    return nullptr;
}

void PartyModeMgr::SetLeftTeamStarBonus() {
    unk90 = 0;
    if (unk54) {
        HamPlayerData *playerData0 = TheGameData->Player(0);
        Hmx::Object *provider0 = playerData0->Provider();
        HamPlayerData *playerData1 = TheGameData->Player(1);
        Hmx::Object *provider1 = playerData1->Provider();
        float f7 = provider0->Property("score")->Float();
        float f8 = provider1->Property("score")->Float();
        if (f8 > f7) {
            int numStars = TheHamProvider->Property("stars_earned", false)->Int();
            if (numStars == 5) {
                unk90 = mEventScoring->FindFloat("five_star_bonus");
            } else if (numStars == 6) {
                unk90 = mEventScoring->FindFloat("six_star_bonus");
            }
        }
    }
}

void PartyModeMgr::SetRightTeamStarBonus() {
    unk94 = 0;
    if (unk54) {
        HamPlayerData *playerData0 = TheGameData->Player(0);
        Hmx::Object *provider0 = playerData0->Provider();
        HamPlayerData *playerData1 = TheGameData->Player(1);
        Hmx::Object *provider1 = playerData1->Provider();
        float f7 = provider0->Property("score")->Float();
        float f8 = provider1->Property("score")->Float();
        if (f7 > f8) {
            int numStars = TheHamProvider->Property("stars_earned", false)->Int();
            if (numStars == 5) {
                unk94 = mEventScoring->FindFloat("five_star_bonus");
            } else if (numStars == 6) {
                unk94 = mEventScoring->FindFloat("six_star_bonus");
            }
        }
    }
}

float PartyModeMgr::GetPointsForWin() {
    static Symbol win("win");
    DataArray *winPoints = mEventScoring->FindArray(win);
    MILO_ASSERT(winPoints, 0x427);
    DataArray *winData = winPoints->FindArray(mCurrEvent->mName, false);
    if (winData) {
        return winData->Float(1);
    } else {
        MILO_NOTIFY(
            "Party mode event %s does not have win scoring data", mCurrEvent->mName.Str()
        );
        return 0;
    }
}

float PartyModeMgr::GetPointsForLoss() {
    static Symbol lose("lose");
    DataArray *losePoints = mEventScoring->FindArray(lose);
    MILO_ASSERT(losePoints, 0x43D);
    DataArray *loseData = losePoints->FindArray(mCurrEvent->mName, false);
    if (loseData) {
        return loseData->Float(1);
    } else {
        MILO_NOTIFY(
            "Party mode event %s does not have lose scoring data", mCurrEvent->mName.Str()
        );
        return 0;
    }
}

void PartyModeMgr::UpdateRoundsPlayed() {
    unk48++;
    if (unk50 == 0) {
        unk50 = unk4c;
    } else {
        unk50--;
    }
    MILO_LOG(
        "----- updating rounds played - rounds played: %d; rounds until showdown: %d\n",
        unk48,
        unk50
    );
}

Symbol PartyModeMgr::GetCurrEventName() {
    MILO_ASSERT(mCurrEvent, 0x4BC);
    Symbol ret(gNullStr);
    ret = mCurrEvent->mName;
    return ret;
}

Symbol PartyModeMgr::GetCurrEventMicrogameName() {
    MILO_ASSERT(mCurrEvent, 0x4CB);
    Symbol ret(gNullStr);
    ret = mCurrEvent->mMicrogameName;
    return ret;
}

Symbol PartyModeMgr::GetCurrEventSongName() {
    MILO_ASSERT(mCurrEvent, 0x4D4);
    Symbol ret(gNullStr);
    const HamSongMetadata *data = TheHamSongMgr.Data(mCurrEvent->mSongID);
    MILO_ASSERT(data, 0x4DB);
    ret = Symbol(data->Title());
    return ret;
}

Symbol PartyModeMgr::GetCurrEventSongShortName() {
    MILO_ASSERT(mCurrEvent, 0x4E3);
    Symbol ret(gNullStr);
    const HamSongMetadata *data = TheHamSongMgr.Data(mCurrEvent->mSongID);
    MILO_ASSERT(data, 0x4EA);
    SongRecord record(data);
    ret = record.ShortName();
    return ret;
}

Symbol PartyModeMgr::GetCurrEventSongArtistName() {
    MILO_ASSERT(mCurrEvent, 0x4F3);
    Symbol ret(gNullStr);
    static Symbol partymode_intermission("partymode_intermission");
    if (mCurrEvent->mName == partymode_intermission) {
        return ret;
    } else {
        const HamSongMetadata *data = TheHamSongMgr.Data(mCurrEvent->mSongID);
        MILO_ASSERT(data, 0x500);
        ret = Symbol(data->Artist());
    }
    return ret;
}

int PartyModeMgr::GetCurrEventPlayerFlags() {
    MILO_ASSERT(mCurrEvent, 0x508);
    return mCurrEvent->mPlayerFlags;
}

int PartyModeMgr::GetCurrEventNumPlayers() {
    MILO_ASSERT(mCurrEvent, 0x511);
    return mCurrEvent->mNumPlayers;
}

DataArray *PartyModeMgr::GetCurrEventPlayers() {
    MILO_ASSERT(mCurrEvent, 0x51A);
    return mCurrEvent->mPlayers;
}

void PartyModeMgr::SetupCharacterData() {
    MILO_ASSERT(TheHamProvider->Property("is_in_party_mode")->Int(), 0x5DB);
    for (int i = 0; i < 2; i++) {
        HamPlayerData *hpd = TheGameData->Player(i);
        Symbol crew;
        if (hpd->Side() == kSkeletonRight) {
            crew = unk1bc;
        } else {
            crew = unk1b8;
        }
        hpd->SetCrew(crew);
        Symbol crewChar = GetCrewCharacter(crew, rand() % GetNumCrewCharacters(crew));
        hpd->SetCharacter(crewChar);
        Symbol outfit = GetCrewLookOutfit(crewChar);
        hpd->SetCharacterOutfit(outfit);
    }
    const HamSongMetadata *pData = TheHamSongMgr.Data(
        TheHamSongMgr.GetSongIDFromShortName(TheGameData->GetSong(), false)
    );
    MILO_ASSERT(pData, 0x5F8);
    TheGameData->SetVenue(pData->Venue());
}

void PartyModeMgr::SmoothFrameMotion(int frame_idx, float f2, float f3) {
    MILO_ASSERT_RANGE(frame_idx, 0, 6, 0x64E);
    unk1e0[frame_idx].Smooth(Vector2(f2, f3), TheTaskMgr.DeltaUISeconds(), false);
}

void PartyModeMgr::ForceFrameSmootherPos(int frame_idx, float f2, float f3) {
    MILO_ASSERT_RANGE(frame_idx, 0, 6, 0x656);
    unk1e0[frame_idx].ForceValue(Vector2(f2, f3));
}

const char *PartyModeMgr::GetPlaylistString() {
    if (!unk2d4) {
        return gNullStr;
    } else {
        String str;
        if (unk2d4->IsCustom()) {
            str = unk2d4->GetName();
        } else {
            str = MakeString("%s_title", unk2d4->GetName());
        }
        const char *fmt = FormatTimeMS(unk2d4->GetDuration());
        static Symbol songname_duration("songname_duration");
        str = MakeString(
            Localize(songname_duration, nullptr, TheLocale),
            Localize(str.c_str(), nullptr, TheLocale),
            fmt
        );
        return str.c_str();
    }
}

DataArray *PartyModeMgr::GetLeftCrewColor1AsArray() {
    static Symbol TEAM_COLORS("TEAM_COLORS");
    DataArray *pTeamArray = DataGetMacro(TEAM_COLORS);
    MILO_ASSERT(pTeamArray, 0x803);
    static Symbol left("left");
    DataArray *pTeamData = pTeamArray->FindArray(left);
    MILO_ASSERT(pTeamData, 0x807);
    static Symbol colors("colors");
    DataArray *pTeamColors = pTeamData->FindArray(colors);
    MILO_ASSERT(pTeamColors, 0x80B);
    DataArray *pTeamColor = pTeamColors->Array(1);
    MILO_ASSERT(pTeamColor, 0x80E);
    return pTeamColor;
}

DataArray *PartyModeMgr::GetLeftCrewColor2AsArray() {
    static Symbol TEAM_COLORS("TEAM_COLORS");
    DataArray *pTeamArray = DataGetMacro(TEAM_COLORS);
    MILO_ASSERT(pTeamArray, 0x817);
    static Symbol left("left");
    DataArray *pTeamData = pTeamArray->FindArray(left);
    MILO_ASSERT(pTeamData, 0x81B);
    static Symbol colors("colors");
    DataArray *pTeamColors = pTeamData->FindArray(colors);
    MILO_ASSERT(pTeamColors, 0x81F);
    DataArray *pTeamColor = pTeamColors->Array(2);
    MILO_ASSERT(pTeamColor, 0x822);
    return pTeamColor;
}

DataArray *PartyModeMgr::GetRightCrewColor1AsArray() {
    static Symbol TEAM_COLORS("TEAM_COLORS");
    DataArray *pTeamArray = DataGetMacro(TEAM_COLORS);
    MILO_ASSERT(pTeamArray, 0x82B);
    static Symbol right("right");
    DataArray *pTeamData = pTeamArray->FindArray(right);
    MILO_ASSERT(pTeamData, 0x82F);
    static Symbol colors("colors");
    DataArray *pTeamColors = pTeamData->FindArray(colors);
    MILO_ASSERT(pTeamColors, 0x833);
    DataArray *pTeamColor = pTeamColors->Array(1);
    MILO_ASSERT(pTeamColor, 0x836);
    return pTeamColor;
}

DataArray *PartyModeMgr::GetRightCrewColor2AsArray() {
    static Symbol TEAM_COLORS("TEAM_COLORS");
    DataArray *pTeamArray = DataGetMacro(TEAM_COLORS);
    MILO_ASSERT(pTeamArray, 0x83F);
    static Symbol right("right");
    DataArray *pTeamData = pTeamArray->FindArray(right);
    MILO_ASSERT(pTeamData, 0x843);
    static Symbol colors("colors");
    DataArray *pTeamColors = pTeamData->FindArray(colors);
    MILO_ASSERT(pTeamColors, 0x847);
    DataArray *pTeamColor = pTeamColors->Array(2);
    MILO_ASSERT(pTeamColor, 0x84A);
    return pTeamColor;
}

Symbol PartyModeMgr::GetLeftCrewCharOutfit(int char_idx, int outfit_idx) {
    int numCrewChars = GetNumCrewCharacters(unk1b8);
    MILO_ASSERT(char_idx < numCrewChars, 0x877);
    if (char_idx < 0) {
        char_idx = rand() % numCrewChars;
    }
    Symbol charSym = GetCrewCharacter(unk1b8, char_idx);
    int numCharOutfits = GetNumCharacterOutfits(charSym);
    MILO_ASSERT(outfit_idx < numCharOutfits, 0x880);
    if (outfit_idx < 0) {
        outfit_idx = rand() % numCharOutfits;
    }
    return GetCharacterOutfit(charSym, outfit_idx);
}

Symbol PartyModeMgr::GetRightCrewCharOutfit(int char_idx, int outfit_idx) {
    int numCrewChars = GetNumCrewCharacters(unk1bc);
    MILO_ASSERT(char_idx < numCrewChars, 0x88E);
    if (char_idx < 0) {
        char_idx = rand() % numCrewChars;
    }
    Symbol charSym = GetCrewCharacter(unk1bc, char_idx);
    int numCharOutfits = GetNumCharacterOutfits(charSym);
    MILO_ASSERT(outfit_idx < numCharOutfits, 0x897);
    if (outfit_idx < 0) {
        outfit_idx = rand() % numCharOutfits;
    }
    return GetCharacterOutfit(charSym, outfit_idx);
}

void PartyModeMgr::ChangeToAnotherGameMode() {
    int i1 = (GetEnumFromModeName(mCurrEvent->mName) + 1) % 5;
    Symbol name = GetModeNameFromEnum(i1);
    while (!IsModeIncluded(name)) {
        i1 = (i1 + 1) % 5;
        name = GetModeNameFromEnum(i1);
    }
    mCurrEvent->mName = name;
}

void PartyModeMgr::EndPartyStats() {
    DateTime dt;
    GetDateAndTime(dt);
    unsigned int diff = dt.DiffSeconds(unk31b);
    for (int i = 0; i < 2; i++) {
        HamPlayerData *playerData = TheGameData->Player(i);
        MILO_ASSERT(playerData, 0x9DA);
        HamProfile *profile = TheProfileMgr.GetProfileFromPad(playerData->PadNum());
        if (profile && profile->GetMetagameStats()) {
            profile->GetMetagameStats()->UpdatePartyStats(diff);
        }
    }
}

void PartyModeMgr::StorePlayerFramePos(int player, float f2, float f3) {
    MILO_ASSERT(player >= 0 && player < mPlayers.size(), 0x369);
    mPlayers[player]->StoreFramePos(f2, f3);
}

void PartyModeMgr::StorePlayerFrameScale(int player, float scale) {
    MILO_ASSERT(player >= 0 && player < mPlayers.size(), 0x370);
    mPlayers[player]->StoreFrameScale(scale);
}

const char *PartyModeMgr::GetPlayerARTexPath(int playerIndex) {
    MILO_ASSERT_RANGE(playerIndex, 0, mPlayers.size(), 0x377);
    return mPlayers[playerIndex]->GetTexPath();
}

void PartyModeMgr::SetRandomCharacters() {
    for (int i = 0; i < 2; i++) {
        HamPlayerData *pPlayerData = TheGameData->Player(i);
        MILO_ASSERT(pPlayerData, 0x5CA);
        Symbol symRandomCharacter = unk1ac[rand() % unk1ac.size()];
        MILO_ASSERT(symRandomCharacter != gNullStr, 0x5CE);
        Symbol crew = GetCrewForCharacter(symRandomCharacter);
        pPlayerData->SetCharacter(symRandomCharacter);
        pPlayerData->SetCrew(crew);
        pPlayerData->SetOutfit(GetCharacterOutfit(symRandomCharacter, 0));
    }
}

int PartyModeMgr::GetPlayerPhotoIndex(int player) {
    MILO_ASSERT_RANGE(player, 0, mPlayers.size(), 0x5FF);
    return mPlayers[player]->GetPhotoIndex();
}

void PartyModeMgr::BroadcastSyncMsg(Symbol msgType) {
    MILO_LOG("[PartyModeMgr::BroadcastSyncMsg] Broadcasting msg (%s).\n", msgType.Str());
    Message msg(msgType);
    HandleType(msg);
    TheUI->Handle(msg, false);
}

void PartyModeMgr::SendPartyOptionsToRC() {
    HamProfile *profile = GetValidProfile();
    if (!profile) {
        BroadcastSyncMsg("skipped_sync");
    } else {
        unk2f4 = new SetPartyOptionsJob(this, profile->GetOnlineID()->ToString());
        TheRockCentral.ManageJob(unk2f4);
    }
}

void PartyModeMgr::GetPartyOptionsFromRC() {
    HamProfile *profile = GetValidProfile();
    if (!profile) {
        BroadcastSyncMsg("skipped_sync");
    } else {
        unk2f8 = new GetPartyOptionsJob(this, profile->GetOnlineID()->ToString());
        TheRockCentral.ManageJob(unk2f8);
    }
}

void PartyModeMgr::ReadPartyOptions() {
    unk2f8->GetOptions();
    unk2f8 = nullptr;
    BroadcastSyncMsg("options_updated");
}

void PartyModeMgr::GetPartySongQueueFromRC() {
    HamProfile *profile = GetValidProfile();
    if (!profile) {
        BroadcastSyncMsg("skipped_sync");
    } else {
        unk2fc = new GetPartySongQueueJob(this, profile->GetOnlineID()->ToString());
        TheRockCentral.ManageJob(unk2fc);
    }
}

void PartyModeMgr::DeleteSongFromRCPartySongQueue(int songID) {
    HamProfile *profile = GetValidProfile();
    if (!profile) {
        BroadcastSyncMsg("skipped_sync");
    } else {
        unk304 = new DeleteSongFromPartySongQueueJob(
            this, profile->GetOnlineID()->ToString(), songID
        );
        TheRockCentral.ManageJob(unk304);
    }
}

void PartyModeMgr::AddNextSongToRCPartySongQueue() {
    HamProfile *profile = GetValidProfile();
    if (!profile) {
        BroadcastSyncMsg("skipped_sync");
    } else {
        unk300 = new AddSongToPartySongQueueJob(
            this, profile->GetOnlineID()->ToString(), unk308.front().mSongID
        );
        TheRockCentral.ManageJob(unk300);
    }
}

Symbol PartyModeMgr::GetNextMode() {
    MILO_ASSERT(mModePicker.Size() > 0, 0x17B);
    return mModePicker.GetNext();
}

void PartyModeMgr::DetermineSubMode(Symbol *s1, Symbol *s2) {
    if (mUsePlaytestData) {
        *s1 = mModePicker.GetNext();
        *s2 = unk120.GetNext();
    } else if (TheHamProvider->Property("is_in_party_mode")->Int() && !unk50) {
        static Symbol showdown("showdown");
        static Symbol ffa("ffa");
        *s1 = showdown;
        *s2 = ffa;
    } else {
        *s1 = mModePicker.GetNext();
        if (unk328) {
            Symbol sym = unk328->Sym(unk48 + 1);
            static Symbol event_buckets("event_buckets");
            DataArray *arr = mPartyModeCfg->FindArray(event_buckets);
            arr = arr->FindArray(sym);
            int i12 = 0;
            for (int i = 1; i < arr->Size(); i++) {
                DataArray *curArr = arr->Array(i);
                if (IsModeIncluded(curArr->Sym(0))) {
                    i12 += curArr->Int(1);
                }
            }
            i12 = rand() % i12;
            int i = 1;
            int i4 = 0;
            for (; i < arr->Size(); i++) {
                DataArray *curArr = arr->Array(i);
                if (IsModeIncluded(curArr->Sym(0))) {
                    i4 += curArr->Int(1);
                    if (i12 < i4) {
                        *s1 = curArr->Sym(0);
                        break;
                    }
                }
            }
        }
        static Symbol dance_battle("dance_battle");
        if (*s1 == dance_battle) {
            static Symbol ffa("ffa");
            *s2 = ffa;
        }
    }
}

void PartyModeMgr::DetermineSubModeSong(Symbol *pShortName, int *pSongID) {
    if (unk324 && !unk2d4) {
        DataArray *arr = unk324->Array(unk48 + 1);
        if (arr) {
            int rank = arr->Int(rand() % arr->Size());
            MILO_ASSERT_FMT(
                rank >= 1 && rank <= 4, "%d is an invalid DJ logic intensity rank\n", rank
            );
            *pShortName = unk15c[rank].GetNext();
            *pSongID = TheHamSongMgr.GetSongIDFromShortName(*pShortName);
            return;
        } else {
            MILO_NOTIFY(
                "DJ logic data doesn't contain enough information for %d rounds, picking random song instead",
                unk48
            );
        }
    }
    *pShortName = unkf8.GetNext();
    *pSongID = TheHamSongMgr.GetSongIDFromShortName(*pShortName);
}

bool PartyModeMgr::IsTeamSignedIn(int i1) {
    if (i1 == 1) {
        return unkd0.Size() > 0;
    } else if (i1 == 2) {
        return unke4.Size() > 0;
    } else {
        return false;
    }
}

PartyModePlayer *PartyModeMgr::CreatePartyModePlayer() {
    int objIdx = mPlayers.size() % unkb0.size() + 1;
    DataArray *objArr = mARObjects->Array(objIdx);
    PartyModeARObject *arObj = new PartyModeARObject(objArr);
    PartyModePlayer *player = new PartyModePlayer(arObj);
    player->SetSym(unk1ac[rand() % unk1ac.size()]);
    player->SetIndex(mPlayers.size());
    if (unkd0.Size() <= 0) {
        player->SetPhotoIndex(unk68.size());
    } else {
        player->SetPhotoIndex(unk74.size() + 4);
    }
    return player;
}

void PartyModeMgr::AddPlayerToTeam(int i1) {
    PartyModePlayer *player = CreatePartyModePlayer();
    mPlayers.push_back(player);
    if (i1 == 1) {
        unk68.push_back(player);
    } else if (i1 == 2) {
        unk74.push_back(player);
    }
}

void PartyModeMgr::ClearTeam(int team) {
    switch (team) {
    case 1:
        for (int i = 0; i != unk68.size(); i++) {
            delete unk68[i];
        }
        unk68.clear();
        break;
    case 2:
        for (int i = 0; i != unk74.size(); i++) {
            delete unk74[i];
        }
        unk74.clear();
        break;
    default:
        MILO_ASSERT(team == 1 || team == 2, 0x20F);
        break;
    }
}

void PartyModeMgr::ResetPlayers() {
    for (int i = 0; i < mPlayers.size(); i++) {
        RELEASE(mPlayers[i]);
    }
    mPlayers.clear();
    unk68.clear();
    unk74.clear();
    unkd0.Clear();
    unke4.Clear();
    unk1c0 = nullptr;
    unk1c4 = nullptr;
}

void PartyModeMgr::ResetMicrogames() {
    unk120.Clear();
    DataArray *gamesArr = mPartyModeCfg->FindArray("party_mode_microgames");
    for (int i = 1; i < gamesArr->Size(); i++) {
        unk120.AddItem(gamesArr->Sym(i));
    }
    unk120.Randomize();
}

int PartyModeMgr::PickNextPlayer() {
    int ret = -1;
    if (unk1c8 == 2) {
        ret = unkd0.GetNext();
        if (mUsePlaytestData) {
            ret = ret % unk68.size();
        }
        unk1c8 = 1;
        if (unk32c) {
            DataArray *arr = unk32c->Array(unk48 + 1);
            int idx = 0;
            if (unk68.size() > unk74.size())
                idx = 1;
            ret = arr->Int(idx);
        }
    } else if (unk1c8 == 1) {
        ret = unke4.GetNext();
        if (mUsePlaytestData) {
            ret = ret % unk74.size() + unk68.size();
        }
        unk1c8 = 2;
        if (unk32c) {
            DataArray *arr = unk32c->Array(unk48 + 1);
            int idx = 1;
            if (unk74.size() < unk68.size())
                idx = 0;
            ret = unk68.size() + arr->Int(idx);
        }
    }
    return ret;
}

void PartyModeMgr::ShufflePlaylist(bool b1) {
    MILO_ASSERT(IsUsingPlaylist(), 0x731);
    if (b1) {
        // unkf8.unk8 = 2;
        // unkf8.unk10 = 0;
    } else if (unk2d8) {
        // unkf8.unk8 = 0;
        SetSongsFromPlaylist();
    }
    unk2d8 = b1;
}

void PartyModeMgr::ResetParty() {
    unk48 = 0;
    unk54 = false;
    unk1c8 = 2;
    ResetPlayers();
    unk2d0 = DefaultDifficulty();
    if (unk1d4.empty()) {
        TheHamSongMgr.GetRandomlySelectableRankedSongs(unk1d4);
    }
    unk80 = unk84;
    unk88 = unk8c;
    unk84 = 0;
    unk8c = 0;
    unk2e4 = 0;
    unk2e8 = 0;
    Symbol crew(gNullStr);
    HamPlayerData *pPlayerData = TheGameData->Player(0);
    MILO_ASSERT(pPlayerData, 0x17F);
    pPlayerData->SetCrew(crew);
    pPlayerData = TheGameData->Player(1);
    MILO_ASSERT(pPlayerData, 0x182);
    pPlayerData->SetCrew(crew);
    unk9c = 2;
    unka0 = 2;
    unk324 = nullptr;
}
