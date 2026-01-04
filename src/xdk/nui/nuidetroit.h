#pragma once
#include "../win_types.h"
#include "xdk/XAPILIB.h"
#include "vectorintrinsics.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 16)
typedef struct _NUI_TILT_OBJECTS { /* Size=0xd0 */
    /* 0x0000 */ FLOAT CameraHeightMeters;
    /* 0x0004 */ DWORD Count;
    /* 0x0010 */ struct {
        /* 0x0000 */ XMVECTOR Location;
        /* 0x0010 */ DWORD Flags;
    } Objects[6];
} NUI_TILT_OBJECTS;
#pragma pack(pop)

typedef unsigned int NUI_TILT_FLAGS;

DWORD NuiCameraAdjustTilt(
    NUI_TILT_FLAGS TiltFlags,
    FLOAT SpaceAboveHeadMeters,
    FLOAT FarSpaceDistanceMeters,
    FLOAT PreferredPlayspaceDistanceMeters,
    NUI_TILT_OBJECTS *pTiltObjects,
    XOVERLAPPED *pOverlapped
);
HRESULT NuiCameraElevationSetAngle(LONG lAngleDegrees);

#ifdef __cplusplus
}
#endif
