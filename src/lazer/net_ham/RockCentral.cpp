#include "net_ham/RockCentral.h"
#include "meta/ConnectionStatusPanel.h"
#include "net/DingoSvr.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "utl/DataPointMgr.h"
#include "utl/Symbol.h"

char *g_szMachineIdString;
const String RockCentral::kServerVer = "1";
RockCentral TheRockCentral;

namespace {
    void RockCentralTerminate() { TheRockCentral.Terminate(); }

    class DataPointJob {
    public:
        DataPointJob(DataPoint &, String &);
    };

    void SendDataPointNoReturn(DataPoint &);
}

RockCentral::RockCentral()
    : mState(), unk7c(0), unk80(0), unk84(60000), unk88(-1), unk8c(0), unk90(0), unkd8(0),
      mLoginBlocked(0), unkdd(0), unk120(0), unk124(0), unk128(0), unk12c(0) {}

RockCentral::~RockCentral() {}

BEGIN_HANDLERS(RockCentral)
    HANDLE_MESSAGE(ServerStatusChangedMsg)
    HANDLE_MESSAGE(ConnectionStatusChangedMsg)
    HANDLE_MESSAGE(TmsDownloadedMsg)
    HANDLE_MESSAGE(RCJobCompleteMsg)
    HANDLE_MESSAGE(UserLoginMsg)
    HANDLE_EXPR(state, mState)
    HANDLE_ACTION(force_logout, ForceLogout())
    HANDLE_EXPR(is_online, IsOnline())
    HANDLE_EXPR(toggle_block_login, mLoginBlocked = !mLoginBlocked)
    HANDLE_EXPR(is_login_blocked, mLoginBlocked)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void RockCentral::ForceLogout() {
    if (mState == 2 || mState == 1) {
        mState = (State)3;
        TheServer.Logout();
    }
}

bool RockCentral::IsOnline() {
    if (mLoginBlocked) {
        return false;
    } else {
        return mState == 2;
    }
}

void RockCentral::SetLoginName(const char *name) { TheServer.SetLoginName(name); }
void RockCentral::SetLoginPassword(const char *password) {
    TheServer.SetLoginPassword(password);
}

void RockCentral::Login() {
    mState = (State)1;
    unkdd = false;
    if (!TheServer.Authenticate(0)) { // should be TheServer->unk74
        Export(ServerStatusChangedMsg((ServerStatusResult)4), false);
    }
}

void RockCentral::CreateAccount() {
    MILO_ASSERT(mState == kFailed, 0x124);
    TheServer.CreateAccount();
}

void RockCentral::OnJobFinished(RCJob *job) {
    MILO_ASSERT(job->IsFinished(), 0x24A);
    delete job;
}

void RockCentral::Init() {
    SetName("rock_central", ObjectDir::Main());
    TheServer.AddSink(this);
    static Symbol connection_status_changed("connection_status_changed");
    ThePlatformMgr.AddSink(this, ConnectionStatusChangedMsg::Type());
    ThePlatformMgr.AddSink(this, TmsDownloadedMsg::Type());
    TheDebug.AddExitCallback(RockCentralTerminate);
    TheDataPointMgr.SetDataPointRecorder(SendDataPointNoReturn);
    unke0.Generate();
    unk48.Start();
    unk78 = unk48.Ms();
    unk7c = unk48.Ms() + 600000.0f;
    mDLCMsg = gNullStr;
    mUtilityMsg = gNullStr;
    mCommunityMsgs.clear();
}

void RockCentral::Terminate() {
    TheServer.RemoveSink(this);
    ThePlatformMgr.RemoveSink(this, ConnectionStatusChangedMsg::Type());
    ThePlatformMgr.RemoveSink(this, TmsDownloadedMsg::Type());
}

void RockCentral::GetCommunityMsg(int index, String &str) const {
    MILO_ASSERT_RANGE(index, 0, mCommunityMsgs.size(), 0x1AE);
    str = mCommunityMsgs[index];
}

int RockCentral::GetCommunityMsgCount() const { return mCommunityMsgs.size(); }
bool RockCentral::HasDlcMsg() { return !(mDLCMsg == gNullStr); }
void RockCentral::GetDlcMsg(String &str) const { str = mDLCMsg; }
bool RockCentral::HasUtilityMsg() { return !(mUtilityMsg == gNullStr); }
void RockCentral::GetUtilityMsg(String &str) const { str = mUtilityMsg; }
DataNode RockCentral::OnMsg(const UserLoginMsg &) { return 1; }

void RockCentral::ManageJob(RCJob *job) {
    if (!mLoginBlocked) {
        TheServer.ManageJob(job);
    }
}
