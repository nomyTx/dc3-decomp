#pragma once
#include <types.h>

extern u32 gMainThreadID;

void ThreadCallTerminate();
void ThreadCallPreInit();
void ThreadCallPoll();
void ThreadCallInit();

class ThreadCallback {
public:
    ThreadCallback() {}
    virtual ~ThreadCallback() {}
    virtual int ThreadStart() = 0;
    virtual void ThreadDone(int) = 0;
};

typedef int ThreadCallFunc(void);
typedef void ThreadCallCallbackFunc(int);

void ThreadCall(ThreadCallFunc *, ThreadCallCallbackFunc *);
void ThreadCall(ThreadCallback *);
