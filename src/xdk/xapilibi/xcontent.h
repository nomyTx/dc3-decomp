#pragma once
#include "../win_types.h"
#include "xdk/xapilibi/xuser.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _XDEVICE_DATA { /* Size=0x50 */
    /* 0x0000 */ DWORD DeviceID;
    /* 0x0004 */ DWORD DeviceType;
    /* 0x0008 */ ULONGLONG ulDeviceBytes;
    /* 0x0010 */ ULONGLONG ulDeviceFreeBytes;
    /* 0x0018 */ wchar_t wszFriendlyName[27];
};
typedef _XDEVICE_DATA XDEVICE_DATA;

DWORD XContentClose(const char *, XOVERLAPPED *);
DWORD XContentGetDeviceData(DWORD, XDEVICE_DATA *);

#define XCONTENT_MAX_FILENAME_LENGTH 42
#define XCONTENT_MAX_DISPLAYNAME_LENGTH 128

struct _XCONTENT_CROSS_TITLE_DATA { /* Size=0x138 */
    /* 0x0000 */ DWORD DeviceID;
    /* 0x0004 */ DWORD dwContentType;
    /* 0x0008 */ wchar_t szDisplayName[XCONTENT_MAX_DISPLAYNAME_LENGTH];
    /* 0x0108 */ char szFileName[XCONTENT_MAX_FILENAME_LENGTH];
    /* 0x0134 */ DWORD dwTitleId;
};
typedef _XCONTENT_CROSS_TITLE_DATA XCONTENT_CROSS_TITLE_DATA;

#ifdef __cplusplus
}
#endif
