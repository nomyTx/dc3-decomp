#include "game/GamePanel.h"
#include "game/Game.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Overlay.h"
#include "ui/UIPanel.h"

GamePanel *TheGamePanel;

GamePanel::GamePanel()
    : unk38(0), unk6c(RndOverlay::Find("time")), unk70(RndOverlay::Find("latency")),
      unk74(RndOverlay::Find("fitness")), unk78(RndOverlay::Find("loop_viz")), unk7c(0),
      unk80(0), unk84(0), unk88("game_panel_load", 1), unkd8(0), unke8(0), unkec(-2),
      unkf0(0), unkf8(1), unkfc(new Timer()), unk100(1), unk101(0), unk104(0), unk108(0) {
    mFitnessFilters[0].SetPlayerIndex(0);
    mFitnessFilters[1].SetPlayerIndex(1);
    unkdc.resize(32);
    // set two globals/statics
    MILO_ASSERT(!TheGamePanel, 0x9E);
    TheGamePanel = this;
    SetType("none");
}

GamePanel::~GamePanel() {
    TheGamePanel = nullptr;
    RELEASE(unkfc);
}

BEGIN_HANDLERS(GamePanel)
    HANDLE_ACTION(set_start_paused, unk7c = _msg->Int(2))
    HANDLE_EXPR(in_intro, unk80 == 1)
    HANDLE_EXPR(is_game_over, unk80 == 3)
    HANDLE_EXPR(is_playing, unk80 == 2)
    HANDLE_ACTION(start_game, StartGame())
    HANDLE(start_load_song, OnStartLoadSong)
    HANDLE(start_song_now, OnStartSongNow)
    HANDLE_ACTION(set_paused_except_sound, SetPausedHelper(_msg->Int(2), false))
    HANDLE_ACTION(cheat_pause, CheatPause(_msg->Int(2)))
    HANDLE_ACTION(clear_draw_glitch, ClearDrawGlitch())
    HANDLE_ACTION(reload_data, ReloadData())
    HANDLE_ACTION(win, SetGameOver(true))
    HANDLE_EXPR(is_past_stream_jump_point_of_no_return, IsPastStreamJumpPointOfNoReturn())
    HANDLE_ACTION(reset_limb_feedback, ResetLimbFeedback())
    HANDLE_ACTION(set_limb_feedback_visible, SetLimbFeedbackVisible(_msg->Int(2)))
    HANDLE(get_fitness_data, OnGetFitnessData)
    HANDLE_MESSAGE(EndGameMsg)
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS
