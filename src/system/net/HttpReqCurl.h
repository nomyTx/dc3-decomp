#pragma once
#include "curl/curl.h"
#include "net/HttpReq.h"
#include "utl/Str.h"

class HttpReqCurl : public HttpReq {
public:
    HttpReqCurl(ReqType, unsigned int, unsigned short, char const *);
    HttpReqCurl(ReqType, char const *, unsigned short, char const *);
    virtual ~HttpReqCurl();
    virtual void Start();
    virtual void Do();
    virtual void Reset();
    virtual char *DetachBuffer();
    virtual unsigned int GetBufferSize();
    virtual void SetTimeout(unsigned int);
    virtual void SetSSLCertPath(char const *);
    virtual void SetSSLCertName(char const *);
    virtual void SetSSLVerifyPeer(unsigned short);
    virtual void SetSSLVerifyHost(unsigned short);

private:
    curl_slist *mHeaders; // 0x50
    CURL *mReq; // 0x54
    char *mBuffer; // 0x58 - file the output should be written to
    unsigned int mBufferLength; // 0x5c
    String mSSLCertPath; // 0x60
    String mSSLCertName; // 0x68
    unsigned short mSSLVerifyPeer; // 0x70
    unsigned short mSSLVerifyHost; // 0x72
    unsigned int mTimeout; // 0x74
};
