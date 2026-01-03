#pragma once
#include "../win_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NUI_ENROLLMENT_INFORMATION { /* Size=0x8 */
    /* 0x0000 */ DWORD dwUserIndex;
    /* 0x0004 */ DWORD dwEnrollmentFlags;
} NUI_ENROLLMENT_INFORMATION;

HRESULT NuiIdentityAbort();
HRESULT NuiIdentityGetEnrollmentInformation(
    DWORD dwEnrollmentIndex, NUI_ENROLLMENT_INFORMATION *pEnrollmentInformation
);

#ifdef __cplusplus
}
#endif
