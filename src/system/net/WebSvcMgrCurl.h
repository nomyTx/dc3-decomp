#pragma once
#include "WebSvcReq.h"
#include "curl/multi.h"
#include "net/HttpReq.h"
#include "net/WebSvcMgr.h"

class WebSvcMgrCurl : public WebSvcMgr {
public:
    WebSvcMgrCurl() : mCurlMultiHandle(0) {}
    virtual ~WebSvcMgrCurl();
    virtual void Init();
    virtual void Poll();
    virtual bool DoRequest(
        ReqType,
        unsigned int,
        unsigned short,
        const char *,
        const char *,
        unsigned int,
        const char *,
        unsigned int
    );
    virtual bool InitRequest(
        WebSvcRequest *, ReqType, const char *, unsigned short, const char *, unsigned int
    );
    virtual bool InitRequest(
        WebSvcRequest *, ReqType, unsigned int, unsigned short, const char *, unsigned int
    );

    void InitCurl();

private:
    void FindAndFinish(void *, bool, unsigned int);

    bool InitRequest(
        WebSvcRequest *,
        ReqType,
        const char *,
        unsigned int,
        unsigned short,
        const char *,
        unsigned int
    );

protected:
    virtual void Start(WebSvcRequest *);

    CURLM *mCurlMultiHandle; // 0x4c
};

extern WebSvcMgrCurl gWebSvcMgr;
