#pragma once
#include "../win_types.h"
#include "xdk/xapilibi/xuser.h"

#ifdef __cplusplus
extern "C" {
#endif

DWORD XGetOverlappedExtendedError(XOVERLAPPED *);

#ifdef __cplusplus
}
#endif
