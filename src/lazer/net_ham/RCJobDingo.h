#pragma once
#include "net/DingoJob.h"
#include "obj/Object.h"
#include "obj/Msg.h"

class RCJob : public DingoJob {
public:
    RCJob(const char *, Hmx::Object *);
    virtual ~RCJob();

protected:
    virtual void SendCallback(bool, bool);
};

DECLARE_MESSAGE(RCJobCompleteMsg, "rc_job_complete")
RCJobCompleteMsg(RCJob *job, bool b) : Message(Type(), job, b) {}
END_MESSAGE
