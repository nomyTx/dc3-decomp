#include "net_ham/RCJobDingo.h"
#include "net/DingoJob.h"
#include "obj/Object.h"

RCJob::RCJob(const char *cc, Hmx::Object *o) : DingoJob(cc, o) {}
RCJob::~RCJob() {}

void RCJob::SendCallback(bool b1, bool b2) {
    if (unk34) {
        ParseResponse();
        static RCJobCompleteMsg msg(this, false);
        msg[0] = this;
        msg[1] = b1;
        unk34->Handle(msg, true);
    }
}
