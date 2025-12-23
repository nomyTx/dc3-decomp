#include "net/WebSvcMgrCurl.h"
#include "WebSvcReq.h"
#include "curl/curl.h"
#include "curl/multi.h"
#include "net/HttpReq.h"
#include "net/WebSvcMgr.h"
#include "os/Debug.h"

WebSvcMgrCurl gWebSvcMgr;
WebSvcMgr &TheWebSvcMgr = gWebSvcMgr;

WebSvcMgrCurl::~WebSvcMgrCurl() {
    if (mCurlMultiHandle) {
        MILO_ASSERT(CURLM_OK == curl_multi_cleanup(mCurlMultiHandle), 0x28);
        mCurlMultiHandle = nullptr;
    }
}

void WebSvcMgrCurl::Init() {
    WebSvcMgr::Init();
    InitCurl();
}

void WebSvcMgrCurl::Poll() {
    WebSvcMgr::Poll();
    MILO_ASSERT(mCurlMultiHandle, 0xFE);
    int running_handles;
    if (curl_multi_perform(mCurlMultiHandle, &running_handles) == CURLM_OK) {
        static int sRunningHandles;
        if (sRunningHandles != running_handles) {
            sRunningHandles = running_handles;
        }
        int msgs_in_queue = 0;
        int i5 = 100;
        CURLMsg *msg;
        while (msg = curl_multi_info_read(mCurlMultiHandle, &msgs_in_queue), msg) {
            if (msg->msg == CURLMSG_DONE) {
                CURL *handle = msg->easy_handle;
                bool b6 = false;
                unsigned int arg = 0;
                MILO_ASSERT(handle, 0x11D);
                if (msg->data.result == 0
                    && curl_easy_getinfo(handle, (CURLINFO)0x200002, &arg) == 0
                    && arg == 200) {
                    b6 = true;
                }
                FindAndFinish(handle, b6, arg);
            }
            i5--;
            if (i5 == 0)
                return;
        }
    }
}

bool WebSvcMgrCurl::DoRequest(
    ReqType type,
    unsigned int ip_addr,
    unsigned short port,
    const char *c4,
    const char *c5,
    unsigned int timeout,
    const char *c7,
    unsigned int ui8
) {
    MILO_ASSERT(ip_addr, 0x51);
    MILO_ASSERT(port, 0x52);
    WebSvcRequest req(c4, c5, nullptr);
    InitRequest(&req, type, ip_addr, port, c7, ui8);
    req.Start();
    req.SetTimeout(timeout);
    req.Do();
    return req.HasSucceeded();
}

bool WebSvcMgrCurl::InitRequest(
    WebSvcRequest *req,
    ReqType type,
    unsigned int ip_addr,
    unsigned short port,
    const char *cc,
    unsigned int ui
) {
    MILO_ASSERT(ip_addr, 0x91);
    MILO_ASSERT(port, 0x92);
    MILO_ASSERT(req, 0x93);
    return InitRequest(req, type, nullptr, ip_addr, port, cc, ui);
}

bool WebSvcMgrCurl::InitRequest(
    WebSvcRequest *req,
    ReqType type,
    const char *host_name,
    unsigned short port,
    const char *cc,
    unsigned int ui
) {
    MILO_ASSERT(strlen(host_name), 0xA0);
    MILO_ASSERT(port, 0xA1);
    MILO_ASSERT(req, 0xA2);
    return InitRequest(req, type, host_name, 0, port, cc, ui);
}

void WebSvcMgrCurl::Start(WebSvcRequest *req) {
    WebSvcMgr::Start(req);
    MILO_ASSERT(req->GetRequest(), 0x40);
    CURL *curl_req = static_cast<CURL *>(req->GetRequest());
    MILO_ASSERT(CURLM_OK == curl_multi_add_handle(mCurlMultiHandle, curl_req), 0x42);
}

void WebSvcMgrCurl::InitCurl() {
    mCurlMultiHandle = curl_multi_init();
    MILO_ASSERT(mCurlMultiHandle, 0x31);
}
