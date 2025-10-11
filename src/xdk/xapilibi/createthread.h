#pragma once
#include "../win_types.h"

#ifdef __cplusplus
extern "C" {
#endif

HANDLE CreateThread(void *, DWORD, DWORD (*)(HANDLE), void *, DWORD, DWORD *);

#ifdef __cplusplus
}
#endif
