#include "App.h"
#include "ChecksumData_xbox.h"
#include "game/Game.h"
#include "obj/Dir.h"
#include "os/Timer.h"
#include "rndobj/HiResScreen.h"

App::App(int, char **) {
    ObjDirPtr<ObjectDir> dPtr;
    AutoTimer timer(0, 0, 0, 0);
}

void App::CaptureHiRes() {
    bool paused = AllPaused();

    if (paused)
        TheGame->SetTimePaused(true);

    DrawRegular();

    int tiles = TheHiResScreen.GetTiling() * TheHiResScreen.GetTiling();

    for (int i = 0; i <= tiles; i++) {
        DrawRegular();
        TheHiResScreen.Accumulate();
    }

    TheHiResScreen.Finish();

    if (paused)
        TheGame->SetTimePaused(false);
}
