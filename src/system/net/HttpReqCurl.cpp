#include "net/HttpReqCurl.h"
#include "curl/curl.h"
#include "curl/easy.h"
#include "net/HttpReq.h"
#include "os/Debug.h"
#include "os/NetworkSocket.h"
#include "utl/MemMgr.h"
#include "utl/Std.h"

namespace {
    unsigned int WriteMemoryCallback(void *, unsigned int, unsigned int, void *);
}

HttpReqCurl::HttpReqCurl(ReqType rt, unsigned int ui, unsigned short us, char const *c)
    : HttpReq(rt, ui, us, c), mHeaders(0), mReq(0), mSSLVerifyPeer(0), mSSLVerifyHost(0),
      mTimeout(10000) {
    mBuffer = 0;
    mBufferLength = 0;
}

HttpReqCurl::HttpReqCurl(ReqType rt, char const *c1, unsigned short us, char const *c2)
    : HttpReq(rt, c1, us, c2), mHeaders(0), mReq(0), mSSLVerifyPeer(0), mSSLVerifyHost(0),
      mTimeout(10000) {
    mBuffer = 0;
    mBufferLength = 0;
}

HttpReqCurl::~HttpReqCurl() {
    if (mReq) {
        curl_easy_cleanup(mReq);
        mReq = nullptr;
    }
    if (mBuffer) {
        MemFree(mBuffer, __FILE__, 0x6E);
        mBuffer = nullptr;
    }
    mBufferLength = 0;
}

void HttpReqCurl::Start() {
    MILO_ASSERT(!mReq, 0x75);
    mReq = curl_easy_init();
    String str60;
    if (mType == 2 || mType == 3) {
        str60 = "s";
        curl_easy_setopt(mReq, CURLOPT_SSL_VERIFYPEER, mSSLVerifyPeer);
        curl_easy_setopt(mReq, CURLOPT_SSL_VERIFYHOST, mSSLVerifyHost);
    }
    String urlStr;
    if (mIPAddr == 0) {
        MILO_ASSERT(mHostName.size(), 0x8A);
        urlStr = MakeString(
            "http%s://%s:%d%s", str60.c_str(), mHostName.c_str(), mPort, mURL.c_str()
        );
    } else {
        urlStr = MakeString(
            "http%s://%s:%d%s",
            str60.c_str(),
            NetworkSocket::IPIntToString(mIPAddr).c_str(),
            mPort,
            mURL.c_str()
        );
    }
    curl_easy_setopt(mReq, CURLOPT_URL, urlStr.c_str());
    if (mUserAgent.length()) {
        curl_easy_setopt(mReq, CURLOPT_USERAGENT, mUserAgent.c_str());
    }
    curl_easy_setopt(mReq, CURLOPT_ACCEPT_ENCODING, "gzip");
    curl_easy_setopt(mReq, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(mReq, CURLOPT_FILE, &mBuffer);
    curl_easy_setopt(mReq, CURLOPT_COOKIEFILE, "");
    SetTimeout(mTimeout);
    String cookieStr;
    FOREACH (it, mCookies) {
        if (cookieStr.length()) {
            cookieStr += "; ";
        }
        cookieStr += it->first;
        cookieStr += "=";
        cookieStr += it->second;
    }
    if (cookieStr.length()) {
        curl_easy_setopt(mReq, CURLOPT_COOKIE, cookieStr.c_str());
    }
    curl_easy_setopt(mReq, CURLOPT_POSTREDIR, 3);
    curl_easy_setopt(mReq, CURLOPT_FOLLOWLOCATION, 1);
    if (mHeaders) {
        curl_easy_setopt(mReq, CURLOPT_HTTPHEADER, mHeaders);
    }
    switch (mType) {
    case 1:
    case 3:
        break;
    case kHttpReqType_POST:
    case 2:
        curl_easy_setopt(mReq, CURLOPT_POST, 1);
        curl_easy_setopt(mReq, CURLOPT_POSTFIELDS, mContent);
        curl_easy_setopt(mReq, CURLOPT_POSTFIELDSIZE, mContentLength);
        break;
    default:
        MILO_FAIL("Unknown HttpReqCurl type %d.\n", mType);
        break;
    }
}

void HttpReqCurl::Do() {
    MILO_ASSERT(mReq, 0x122);
    int ret = curl_easy_perform(mReq);
    if (ret == 0) {
        unk4c = 3;
    } else {
        unk4c = 4;
    }
}

void HttpReqCurl::Reset() {
    HttpReq::Reset();
    if (mReq) {
        curl_easy_cleanup(mReq);
        mReq = nullptr;
    }
    if (mBuffer) {
        MemFree(mBuffer, __FILE__, 0xEA);
        mBuffer = nullptr;
    }
    mBufferLength = 0;
}

char *HttpReqCurl::DetachBuffer() {
    MILO_ASSERT(mReq, 0x132);
    if (!HasSucceeded()) {
        return nullptr;
    } else {
        char *ret = mBuffer;
        mBuffer = nullptr;
        return ret;
    }
}

unsigned int HttpReqCurl::GetBufferSize() {
    MILO_ASSERT(mReq, 0x141);
    return mBufferLength;
}

void HttpReqCurl::SetTimeout(unsigned int timeout) {
    mTimeout = timeout;
    if (mReq) {
        CURLcode code = curl_easy_setopt(mReq, CURLOPT_TIMEOUT_MS, timeout);
        if (code != CURLE_OK) {
            MILO_NOTIFY("HttpReqCurl::SetTimeout: Unable to set timeout: %d.", code);
        }
    }
}

void HttpReqCurl::SetSSLCertPath(const char *path) {
    MILO_ASSERT(path, 0x100);
    mSSLCertPath = path;
}

void HttpReqCurl::SetSSLCertName(const char *name) {
    MILO_ASSERT(name, 0x108);
    mSSLCertName = name;
}

void HttpReqCurl::SetSSLVerifyPeer(unsigned short value) {
    MILO_ASSERT(value <= 1, 0x110);
    mSSLVerifyPeer = value;
}

void HttpReqCurl::SetSSLVerifyHost(unsigned short value) {
    MILO_ASSERT(value <= 2, 0x118);
    mSSLVerifyHost = value;
}
