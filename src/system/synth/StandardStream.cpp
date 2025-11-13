#include "synth/StandardStream.h"
#include "os/Debug.h"
#include "os/File.h"
#include "synth/ADSR.h"
#include "utl/MemMgr.h"
#include "utl/Std.h"
#include "synth/StreamReceiver.h"
#include "utl/Symbol.h"
#include <functional>

bool StandardStream::sReportLargeTimerErrors = true;

StandardStream::StandardStream(
    File *f, float f1, float f2, Symbol ext, bool b1, bool b2, bool b3
)
    : unk150(b2), unk158(b3) {
    MILO_ASSERT(f, 0x4A);
    mExt = ext;
    mFile = f;
    mInfoChannels = -1;
    unkec = -1;
    Init(f1, f2, ext, false);
}

StandardStream::~StandardStream() {
    RELEASE(mFile);
    for (int i = 0; i < unk10c.size(); i++) {
        delete unk10c[i];
    }
    Destroy();
    DeleteAll(mChanParams);
    FOREACH (it, unkf4) {
        MemFree(*it);
    }
}

bool StandardStream::Fail() { return mRdr && mRdr->Fail(); }
bool StandardStream::IsReady() const {
    return mState == kReady || mState == kPlaying || mState == kStopped;
}
bool StandardStream::IsFinished() const { return mState == kFinished; }
int StandardStream::GetNumChannels() const { return mChannels.size(); }
int StandardStream::GetNumChanParams() const { return mChanParams.size(); }

void StandardStream::Play() {
    if (!IsReady() && mState != kSuspended) {
        MILO_FAIL(
            "StandardStream::Play() failed. IsReady=%d mState=%d", IsReady(), mState
        );
    }
    UpdateVolumes();
    std::for_each(mChannels.begin(), mChannels.end(), std::mem_fun(&StreamReceiver::Play));
    mState = kPlaying;
    mTimer.Start();
}

void StandardStream::Stop() {
    if (mState == kPlaying) {
        std::for_each(
            mChannels.begin(), mChannels.end(), std::mem_fun(&StreamReceiver::Stop)
        );
        mState = kStopped;
        mTimer.Stop();
    }
}

bool StandardStream::IsPlaying() const { return mState == kPlaying; }
bool StandardStream::IsPaused() const { return mState == kStopped; }

void StandardStream::Resync(float f) {
    int bytes;
    while (!mFile->ReadDone(bytes))
        ;
    Destroy();
    mFile->Seek(0, 0);
    float f88 = mJumpFromMs;
    float f8c = mJumpToMs;
    String str94 = mJumpFile;
    Init(f, mBufSecs, mExt, true);
    if (f88 != 0) {
        SetJump(f88, f8c, str94.c_str());
    }
}

void StandardStream::EnableReads(bool b) {
    if (mRdr)
        mRdr->EnableReads(b);
}

float StandardStream::GetTime() {
    if (!mChannels.empty() && mSampleRate != 0) {
        return mLastStreamTime;
    } else
        return mStartMs;
}

// float StandardStream::GetJumpBackTotalTime() { return mAccumulatedLoopbacks; }

float StandardStream::GetInSongTime() {
    float time = GetTime();
    return time + GetJumpBackTotalTime();
}

void StandardStream::SetVolume(int chan, float vol) {
    MILO_ASSERT_RANGE(chan, 0, mChanParams.size(), 0x41E);
    static Symbol _default("_default");
    mChanParams[chan]->mFaders.FindLocal(_default, true)->SetVolume(vol);
}

float StandardStream::GetVolume(int chan) const {
    MILO_ASSERT_RANGE(chan, 0, mChanParams.size(), 0x444);
    return mChanParams[chan]->mFaders.GetVolume();
}

void StandardStream::SetPan(int chan, float pan) {
    MILO_ASSERT_RANGE(chan, 0, mChanParams.size(), 0x426);
    mChanParams[chan]->mPan = pan;
    if (!mChannels.empty()) {
        mChannels[chan]->SetPan(pan);
    }
}

float StandardStream::GetPan(int chan) const {
    MILO_ASSERT_RANGE(chan, 0, mChanParams.size(), 0x44C);
    return mChanParams[chan]->mPan;
}

void StandardStream::SetFXSend(int channel, FxSend *send) {
    MILO_ASSERT_RANGE(channel, 0, mChanParams.size(), 0x4D9);
    mChanParams[channel]->mFxSend = send;
    if (!mChannels.empty()) {
        mChannels[channel]->SetFXSend(send);
    }
}

void StandardStream::SetSpeed(float speed) {
    mSpeed = speed;
    for (int i = 0; i < mChannels.size(); i++) {
        UpdateSpeed(i);
    }
    mTimer.SetSpeed(speed);
}

void StandardStream::LoadMarkerList(const char *cc) {
    ClearMarkerList();
    FileStream stream(cc, FileStream::kRead, 1);
    int i94 = 0;
    stream >> i94;
    int i98 = 0;
    stream >> i98;
    for (int i = 0; i < i98; i++) {
        Marker marker;
        stream >> marker.name;
        stream >> marker.position;
        marker.posMS = ((float)marker.position * 1000.0f) / (float)i94;
        AddMarker(marker);
    }
}

void StandardStream::ClearMarkerList() { mMarkerList.clear(); }
void StandardStream::AddMarker(const Marker &marker) { mMarkerList.push_back(marker); }
int StandardStream::MarkerListSize() const { return mMarkerList.size(); }

bool StandardStream::MarkerAt(int idx, Marker &marker) const {
    if (idx >= MarkerListSize() || idx < 0)
        return false;
    else {
        marker = mMarkerList[idx];
        return true;
    }
}

void StandardStream::SetJump(float fromMs, float toMs, const char *file) {
    MILO_ASSERT(toMs >= 0, 0x2C0);
    MILO_ASSERT(fromMs >= 0 || fromMs == kStreamEndMs, 0x2C1);
    if (IsPastStreamJumpPointOfNoReturn()) {
        MILO_NOTIFY(
            "Trying to set loop points when we're already past the point of no return!"
        );
    }
    mJumpFromMs = fromMs;
    mJumpToMs = toMs;
    mJumpFile = file;
    if (!mJumpFile.empty()) {
        mJumpFile += ".";
        mJumpFile += mExt.Str();
    }
    if (GetSampleRate() == 0)
        mJumpSamplesInvalid = true;
    else
        setJumpSamplesFromMs(fromMs, toMs);
}

void StandardStream::SetJump(String &s1, String &s2) {
    for (int i = 0; i < mMarkerList.size(); i++) {
        if (mMarkerList[i].name == s1) {
            mStartMarker = mMarkerList[i];
        }
        if (mMarkerList[i].name == s2) {
            mEndMarker = mMarkerList[i];
        }
    }
    SetJump(mStartMarker.posMS, mEndMarker.posMS, nullptr);
}

bool StandardStream::CurrentJumpPoints(Marker &start, Marker &end) {
    if (mJumpFromSamples == 0) {
        return false;
    } else {
        start = mStartMarker;
        end = mEndMarker;
        return true;
    }
}

void StandardStream::ClearJump() {
    mJumpFromSamples = 0;
    mJumpFromMs = 0;
    mJumpToSamples = 0;
    mJumpToMs = 0;
}

void StandardStream::EnableSlipStreaming(int channel) {
    MILO_ASSERT(mChannels.empty(), 0x4AF);
    MILO_ASSERT_RANGE(channel, 0, mChanParams.size(), 0x4B0);
    mChanParams[channel]->mSlipEnabled = true;
}

void StandardStream::SetSlipOffset(int channel, float offset) {
    MILO_ASSERT_RANGE(channel, 0, mChanParams.size(), 0x4B7);
    mChannels[channel]->SetSlipOffset(offset);
}

void StandardStream::SlipStop(int channel) {
    MILO_ASSERT_RANGE(channel, 0, mChanParams.size(), 0x4BE);
    mChannels[channel]->SlipStop();
}

float StandardStream::GetSlipOffset(int channel) {
    MILO_ASSERT_RANGE(channel, 0, mChanParams.size(), 0x4CF);
    return mChannels[channel]->GetSlipOffset();
}

void StandardStream::SetSlipSpeed(int channel, float speed) {
    MILO_ASSERT_RANGE(channel, 0, mChanParams.size(), 0x4C5);
    MILO_ASSERT(mChanParams[channel]->mSlipEnabled, 0x4C6);
    mChanParams[channel]->mSlipSpeed = speed;
    if (!mChannels.empty()) {
        UpdateSpeed(channel);
    }
}

FaderGroup &StandardStream::ChannelFaders(int channel) {
    MILO_ASSERT_RANGE(channel, 0, mChanParams.size(), 0x48D);
    return mChanParams[channel]->mFaders;
}

void StandardStream::AddVirtualChannels(int i) {
    MILO_ASSERT(mChannels.empty(), 0x495);
    mVirtualChans += i;
}

void StandardStream::RemapChannel(int i1, int i2) {
    mChanMaps.push_back(std::make_pair(i1, i2));
}

float StandardStream::GetRawTime() {
    float bytes = mChannels[0]->GetBytesPlayed() / 2;
    return (bytes / (float)mSampleRate) * 1000.0f + mStartMs;
}

void StandardStream::SetADSR(int chan, const ADSRImpl &adsr) {
    MILO_ASSERT_RANGE(chan, 0, mChanParams.size(), 0x43A);
    mChanParams[chan]->mADSR = adsr;
    if (!mChannels.empty()) {
        mChannels[chan]->SetADSR(adsr);
    }
}

void StandardStream::SetJumpSamples(int fromSamples, int toSamples, const char *file) {
    MILO_ASSERT(toSamples >= 0, 0x2F6);
    MILO_ASSERT(fromSamples >= 0 || fromSamples == kStreamEndSamples, 0x2F7);
    MILO_ASSERT(file || fromSamples > toSamples || fromSamples == kStreamEndSamples, 0x2F8);
    MILO_ASSERT(mJumpFromSamples == 0, 0x2FA);
    mJumpFromSamples = fromSamples;
    mJumpToSamples = toSamples;
    mJumpFile = file;
    if (!mJumpFile.empty()) {
        mJumpFile += ".";
        mJumpFile += mExt.Str();
    }
    mJumpSamplesInvalid = false;
}

const char *StandardStream::GetSoundDisplayName() {
    if (!IsPlaying()) {
        return gNullStr;
    } else if (mFile) {
        return MakeString("StandardStream: %s", FileGetName(mFile->Filename().c_str()));
    } else {
        return MakeString("StandardStream: --no file--");
    }
}

void StandardStream::SynthPoll() { PollStream(); }
