#pragma once
#include "../win_types.h"
#include "xcontent.h"
#include "xuser.h"

#ifdef __cplusplus
extern "C" {
#endif

DWORD XContentCrossTitleCreate(DWORD, LPCSTR, XCONTENT_CROSS_TITLE_DATA *, DWORD, DWORD *, DWORD *, int, ULONGLONG, XOVERLAPPED *);
DWORD XContentCrossTitleDelete(DWORD, const XCONTENT_CROSS_TITLE_DATA *, XOVERLAPPED *);

#ifdef __cplusplus
}
#endif
