#pragma once
#include "../win_types.h"
#include "xdk/XAPILIB.h"
#include "xdk/D3D9.h"
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

typedef enum _NUI_IMAGE_TYPE {
    NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX = 0x0000,
    NUI_IMAGE_TYPE_COLOR = 0x0001,
    NUI_IMAGE_TYPE_COLOR_YUV = 0x0002,
    NUI_IMAGE_TYPE_DEPTH = 0x0003,
    NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX_IN_COLOR_SPACE = 0x0004,
    NUI_IMAGE_TYPE_DEPTH_IN_COLOR_SPACE = 0x0005,
    NUI_IMAGE_TYPE_COLOR_IN_DEPTH_SPACE = 0x0006,
    NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX_80x60 = 0x0007,
    NUI_IMAGE_TYPE_DEPTH_80x60 = 0x0008,
} NUI_IMAGE_TYPE;

typedef enum _NUI_IMAGE_RESOLUTION {
    NUI_IMAGE_RESOLUTION_80x60 = 0x0000,
    NUI_IMAGE_RESOLUTION_320x240 = 0x0001,
    NUI_IMAGE_RESOLUTION_640x480 = 0x0002,
} NUI_IMAGE_RESOLUTION;

typedef enum _NUI_CAMERA_PROPERTY {
    NUI_CAMERA_PROPERTY_AE_AWB_MODE = 0x0000,
    NUI_CAMERA_PROPERTY_FRAME_RATE_MAX = 0x0001,
    NUI_CAMERA_PROPERTY_AE_FRAME_RATE_MIN = 0x0002,
    NUI_CAMERA_PROPERTY_FLICKER_RATE = 0x0003,
    NUI_CAMERA_PROPERTY_MAXIMUM = 0x0003,
    NUI_CAMERA_PROPERTY_AE_FRAME_RATE_MODE = 0x0002,
} NUI_CAMERA_PROPERTY;

typedef enum _NUI_IMAGE_DIGITALZOOM {
    NUI_IMAGE_DIGITAL_ZOOM_1X = 0x0000,
    NUI_IMAGE_DIGITAL_ZOOM_2X = 0x0001,
} NUI_IMAGE_DIGITALZOOM;

typedef struct _NUI_IMAGE_VIEW_AREA { /* Size=0xc */
    /* 0x0000 */ NUI_IMAGE_DIGITALZOOM eDigitalZoom;
    /* 0x0004 */ LONG lCenterX;
    /* 0x0008 */ LONG lCenterY;
} NUI_IMAGE_VIEW_AREA;

typedef struct _NUI_IMAGE_FRAME { /* Size=0x28 */
    /* 0x0000 */ LARGE_INTEGER liTimeStamp;
    /* 0x0008 */ DWORD dwFrameNumber;
    /* 0x000c */ _NUI_IMAGE_TYPE eImageType;
    /* 0x0010 */ _NUI_IMAGE_RESOLUTION eResolution;
    /* 0x0014 */ D3DTexture *pFrameTexture;
    /* 0x0018 */ DWORD dwFrameFlags;
    /* 0x001c */ _NUI_IMAGE_VIEW_AREA ViewArea;
} NUI_IMAGE_FRAME;

DWORD NuiCameraAdjustTilt(
    NUI_TILT_FLAGS TiltFlags,
    FLOAT SpaceAboveHeadMeters,
    FLOAT FarSpaceDistanceMeters,
    FLOAT PreferredPlayspaceDistanceMeters,
    NUI_TILT_OBJECTS *pTiltObjects,
    XOVERLAPPED *pOverlapped
);
HRESULT NuiCameraElevationGetAngle(LONG *plAngleDegrees, DWORD *pMovingFlags);
HRESULT NuiCameraElevationSetAngle(LONG lAngleDegrees);

HRESULT NuiImageStreamOpen(
    NUI_IMAGE_TYPE eImageType,
    NUI_IMAGE_RESOLUTION eResolution,
    DWORD dwImageFrameFlags,
    DWORD dwFrameLimit,
    HANDLE hNextFrameEvent,
    HANDLE *phStreamHandle
);

HRESULT NuiImageStreamGetNextFrame(
    HANDLE hStream, DWORD dwMillisecondsToWait, const NUI_IMAGE_FRAME **ppcImageFrame
);

HRESULT NuiImageStreamReleaseFrame(HANDLE hStream, const NUI_IMAGE_FRAME *pImageFrame);

#ifdef __cplusplus
}
#endif
