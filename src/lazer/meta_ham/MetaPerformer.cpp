#include "meta_ham/MetaPerformer.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamMove.h"
#include "hamobj/HamNavProvider.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/ProfileMgr.h"
#include "net_ham/RockCentral.h"
#include "obj/Object.h"

MetaPerformer::MetaPerformer(const HamSongMgr &mgr, const char *)
    : mNumRestarts(0), mSongMgr(mgr), unk24(0), mNoFail(0), mGotNewHighScore(0), unk30(0),
      mGotNewBestStars(0), unk32(0), mGotMovesPassedBest(0), mUnlockedNoFlashcards(0),
      mCompletedSongWithNoFlashcards(0), mUnlockedMediumDifficulty(0),
      mUnlockedExpertDifficulty(0), unk38(0), mPracticeOverallScore(0), mMoveScored(0),
      mCheckMoveScored(1), unkbc(0), mPlaylistIndex(0), mPlaylistElapsedTime(0), unke0(0),
      mSkipPracticeWelcome(0), unke3(0) {
    mNumCompleted.reserve(100);
    mSkippedSongs.clear();
    mEnrollmentIndex[0] = -1;
    mEnrollmentIndex[1] = -1;
    unk29 = DateTime(0);
    mSkillsAwards = new SkillsAwardList();
}

MetaPerformer::~MetaPerformer() { delete mSkillsAwards; }

void MetaPerformer::HandleSkippedSong() { mSkippedSongs.insert(mPlaylistIndex); }
void MetaPerformer::HandleSongRestart() { mNumRestarts++; }
bool MetaPerformer::IsGameplayTimerRunning() const { return unk29.ToCode(); }
bool MetaPerformer::GetPlayedLongIntro(Symbol intro) {
    return mLongIntrosPlayed.count(intro) > 0;
}
void MetaPerformer::SetPlayedLongIntro(Symbol intro) { mLongIntrosPlayed.insert(intro); }

BEGIN_HANDLERS(MetaPerformer)
    HANDLE_ACTION(set_venue_pref, TheProfileMgr.SetVenuePreference(_msg->ForceSym(2)))
    HANDLE_EXPR(get_venue_pref, TheProfileMgr.GetVenuePreference())
    HANDLE_EXPR(has_instarank_data, HasInstarankData())
    HANDLE_EXPR(get_instarank, GetInstarank())
    HANDLE_ACTION(reset_songs, ResetSongs())
    HANDLE_EXPR(num_completed, (int)mNumCompleted.size())
    HANDLE_EXPR(
        get_completed_song_id,
        TheHamSongMgr.GetSongIDFromShortName(TheGameData->GetSong(), true)
    )
    HANDLE_EXPR(get_song, GetSong())
    HANDLE_EXPR(is_first_song, mNumCompleted.empty())
    HANDLE_EXPR(is_last_song, IsLastSong())
    HANDLE_EXPR(is_set_complete, IsSetComplete())
    HANDLE_EXPR(is_winning, IsWinning())
    HANDLE_ACTION(handle_skipped_song, HandleSkippedSong())
    HANDLE_ACTION(restart, Restart())
    HANDLE_ACTION(handle_song_restart, HandleSongRestart())
    HANDLE_EXPR(cheat_toggle_finale, sCheatFinale = !sCheatFinale)
    HANDLE_ACTION(
        trigger_song_completion, TriggerSongCompletion(_msg->Int(2), _msg->Float(3))
    )
    HANDLE_ACTION(advance_song, AdvanceSong(_msg->Int(2)))
    HANDLE_EXPR(is_no_fail_active, mNoFail)
    HANDLE_ACTION(use_no_fail, mNoFail = _msg->Int(2))
    HANDLE_EXPR(is_possible_to_fail, !mNoFail)
    HANDLE_EXPR(is_no_flashcards_active, TheProfileMgr.NoFlashcards())
    HANDLE_EXPR(completed_song_with_no_flashcards, mCompletedSongWithNoFlashcards)
    HANDLE_EXPR(get_enrollment_index, mEnrollmentIndex[_msg->Int(2)])
    HANDLE_EXPR(is_gameplay_timer_running, IsGameplayTimerRunning())
    HANDLE_ACTION(jump_gameplay_timer_forward, JumpGameplayTimerForward(_msg->Int(2)))
    HANDLE_EXPR(got_new_high_score, mGotNewHighScore)
    HANDLE_EXPR(got_new_best_stars, mGotNewBestStars)
    HANDLE_EXPR(got_moves_passed_best, mGotMovesPassedBest)
    HANDLE_ACTION(game_init, OnGameInit())
    HANDLE_ACTION(game_reset, OnGameInit())
    HANDLE_ACTION(freestyle_picture_taken, OnFreestylePictureTaken())
    HANDLE_ACTION(
        move_passed,
        OnMovePassed(_msg->Int(2), _msg->Obj<HamMove>(3), _msg->Int(4), _msg->Float(5))
    )
    HANDLE_ACTION(
        practice_move_passed,
        OnPracticeMovePassed(
            _msg->Int(2), _msg->Str(3), (SkillsAward)_msg->Int(4), _msg->Int(5)
        )
    )
    HANDLE_ACTION(
        review_move_passed,
        OnReviewMovePassed(
            _msg->Int(2), _msg->Obj<HamMove>(3), _msg->Int(4), _msg->Float(5)
        )
    )
    HANDLE_EXPR(get_moves_passed, GetMovesPassed(_msg->Int(2)))
    HANDLE_EXPR(get_moves_attempted, mMovesAttempted[_msg->Int(2)])
    HANDLE_ACTION(
        recall_move_passed, OnRecallMovePassed(_msg->Int(2), _msg->Obj<HamMove>(2)) // huh
    )
    HANDLE_EXPR(is_endgame_song, 0)
    HANDLE_EXPR(just_beat_game, mJustBeatGame)
    HANDLE_EXPR(last_played_mode, mLastPlayedMode)
    HANDLE_EXPR(unlocked_no_flashcards, mUnlockedNoFlashcards)
    HANDLE_EXPR(unlocked_medium_difficulty, mUnlockedMediumDifficulty)
    HANDLE_EXPR(unlocked_expert_difficulty, mUnlockedExpertDifficulty)
    HANDLE_EXPR(is_difficulty_unlocked, IsDifficultyUnlocked(_msg->Sym(2)))
    HANDLE_ACTION(calculate_practice_results, CalculatePracticeResults())
    HANDLE_EXPR(get_learn_moves_passed_count, mNumLearnMovesPassed)
    HANDLE_EXPR(get_learn_moves_fastlaned_count, mNumLearnMovesFastLaned)
    HANDLE_EXPR(get_learn_moves_total, mNumLearnMovesTotal)
    HANDLE_EXPR(get_practice_learn_score, mPracticeLearnScore)
    HANDLE_EXPR(get_review_moves_passed_count, mNumReviewMovesPassed)
    HANDLE_EXPR(get_review_moves_total, mNumReviewMovesTotal)
    HANDLE_EXPR(get_practice_review_score, mPracticeReviewScore)
    HANDLE_EXPR(get_practice_overall_score, mPracticeOverallScore)
    HANDLE_EXPR(
        get_practice_overall_moves_passed_count,
        mNumLearnMovesPassed + mNumReviewMovesPassed
    )
    HANDLE_EXPR(
        get_practice_overall_moves_total, mNumLearnMovesTotal + mNumReviewMovesTotal
    )
    HANDLE_EXPR(get_played_long_intro, GetPlayedLongIntro(_msg->Str(2)))
    HANDLE_ACTION(set_played_long_intro, SetPlayedLongIntro(_msg->Str(2)))
    HANDLE_EXPR(get_default_crew_character, GetCrewCharacter(_msg->Sym(2), 0))
    HANDLE_EXPR(get_crew_character, GetCrewCharacter(_msg->Sym(2), _msg->Int(3)))
    HANDLE_EXPR(get_crew_venue, GetCrewVenue(_msg->Sym(2)))
    HANDLE_EXPR(get_crew_for_character, GetCrewForCharacter(_msg->Sym(2)))
    HANDLE_EXPR(is_crew_available, IsCrewAvailable(_msg->Sym(2)))
    HANDLE_EXPR(is_move_scored, mMoveScored)
    HANDLE_ACTION(set_move_scored, mMoveScored = _msg->Int(2))
    HANDLE_EXPR(get_check_move_scored, mCheckMoveScored)
    HANDLE_ACTION(set_check_move_scored, mCheckMoveScored = _msg->Int(2))
    HANDLE_EXPR(has_playlist, HasPlaylist())
    HANDLE_EXPR(is_playlist_empty, IsPlaylistEmpty())
    HANDLE_EXPR(is_playlist_playable, IsPlaylistPlayable())
    HANDLE_EXPR(is_playlist_custom, IsPlaylistCustom())
    HANDLE_ACTION(set_playlist, SetPlaylist(_msg->Sym(2)))
    HANDLE_ACTION(set_default_crews, SetDefaultCrews())
    HANDLE_ACTION(start_playlist, StartPlaylist())
    HANDLE_ACTION(continue_playlist, ContinuePlaylist())
    HANDLE_ACTION(shuffle_playlist, ShufflePlaylist())
    HANDLE_ACTION(repeat_current_playlist_song, RepeatCurrentPlaylistSong())
    HANDLE_EXPR(get_num_songs_in_playlist, GetNumSongsInPlaylist())
    HANDLE_EXPR(get_playlist_index, mPlaylistIndex)
    HANDLE_ACTION(set_playlist_index, SetPlaylistIndex(_msg->Int(2)))
    HANDLE_EXPR(get_playlist_elapsed_time_string, GetPlaylistElapsedTimeString())
    HANDLE_ACTION(
        populate_playlist_song_provider,
        PopulatePlaylistSongProvider(_msg->Obj<HamNavProvider>(2))
    )
    HANDLE_EXPR(get_playlist_name_and_duration, GetPlaylistNameAndDuration())
    HANDLE_EXPR(get_playlist_elapsed_time, mPlaylistElapsedTime)
    HANDLE_ACTION(
        generate_recommended_practice_moves,
        GenerateRecommendedPracticeMoves(_msg->Int(2))
    )
    HANDLE_EXPR(has_recommended_practice_moves, HasRecommendedPracticeMoves()) // fix
    HANDLE_EXPR(failed_any_practice_moves, mSkillsAwards->HasFailure())
    HANDLE_EXPR(get_award, mSkillsAwards->GetAward(_msg->Array(2)))
    HANDLE_ACTION(
        set_award, mSkillsAwards->SetAward(_msg->Array(2), (SkillsAward)_msg->Int(3))
    )
    HANDLE_ACTION(clear_skills_awards, mSkillsAwards->Clear())
    HANDLE_EXPR(should_skip_practice_welcome, mSkipPracticeWelcome)
    HANDLE_ACTION(set_skip_practice_welcome, mSkipPracticeWelcome = _msg->Int(2))
    HANDLE_EXPR(get_num_restarts, mNumRestarts)
    HANDLE_ACTION(send_omg_score_datapoint, SendOmgDatapoint(_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(send_drop_in_datapoint, SendDropInDatapoint(_msg->Int(2)))
    HANDLE_ACTION(send_drop_out_datapoint, SendDropOutDatapoint(_msg->Int(2)))
    HANDLE_ACTION(
        send_speech_datapoint,
        SendSpeechDatapoint(_msg->Array(2), _msg->Float(3), _msg->Sym(4))
    )
    HANDLE_ACTION(get_era, GetEraInvalid())
    HANDLE_MESSAGE(RCJobCompleteMsg)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

bool MetaPerformer::IsSetComplete() const { return mNumCompleted.size() == 1; }
void MetaPerformer::RepeatCurrentPlaylistSong() {
    UpdateSongFromPlaylist();
    UpdateIsLastSong();
}

void MetaPerformer::SetPlaylistIndex(int idx) {
    mPlaylistIndex = idx;
    RepeatCurrentPlaylistSong();
}

bool MetaPerformer::HasRecommendedPracticeMoves() const {
    return mRecommendedPracticeMoves.size() > 0;
}

DataNode MetaPerformer::OnMsg(const RCJobCompleteMsg &) { return 1; }
