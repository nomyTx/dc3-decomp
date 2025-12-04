#pragma once
#include "d3d9types.h"

// Larger struct definitions and functions go here.

#ifdef __cplusplus
extern "C" {
#endif

// source for most if not all of this: the kinect joyride pdb

typedef struct _D3DConstants { /* Size=0x23a0 */
    union {
        /* 0x0000 */ GPUFETCH_CONSTANT Fetch[32];
        struct {
            /* 0x0000 */ GPUTEXTURE_FETCH_CONSTANT TextureFetch[26];
            /* 0x0270 */ GPUVERTEX_FETCH_CONSTANT VertexFetch[18];
        };
    };
    union {
        /* 0x0300 */ XMVECTOR Alu[512];
        struct {
            /* 0x0300 */ XMVECTOR VertexShaderF[256];
            /* 0x1300 */ XMVECTOR PixelShaderF[256];
        };
    };
    union {
        /* 0x2300 */ UINT Flow[40];
        struct {
            /* 0x2300 */ UINT VertexShaderB[4];
            /* 0x2310 */ UINT PixelShaderB[4];
            /* 0x2320 */ UINT VertexShaderI[16];
            /* 0x2360 */ UINT PixelShaderI[16];
        };
    };
} D3DConstants;

#pragma region D3DDevice

typedef struct D3DDevice { /* Size=0x2a80 */
    /* 0x0000 */ _D3DTAGCOLLECTION m_Pending;
    /* 0x0028 */ UINT64 m_Predicated_PendingMask2;
    /* 0x0030 */ UINT *m_pRing;
    /* 0x0034 */ UINT *m_pRingLimit;
    /* 0x0038 */ UINT *m_pRingGuarantee;
    /* 0x003c */ UINT m_ReferenceCount;
    /* 0x0040 */ VOID (*m_SetRenderStateCall[101])(D3DDevice *, UINT);
    /* 0x01d4 */ VOID (*m_SetSamplerStateCall[20])(D3DDevice *, UINT, UINT);
    /* 0x0224 */ UINT (*m_GetRenderStateCall[101])(D3DDevice *);
    /* 0x03b8 */ UINT (*m_GetSamplerStateCall[20])(D3DDevice *, UINT);
    /* 0x0480 */ D3DConstants m_Constants;
    /* 0x2820 */ float m_ClipPlanes[6][4];
    /* 0x2880 */ GPU_DESTINATIONPACKET m_DestinationPacket;
    /* 0x28c0 */ GPU_WINDOWPACKET m_WindowPacket;
    /* 0x28cc */ GPU_VALUESPACKET m_ValuesPacket;
    /* 0x2920 */ GPU_PROGRAMPACKET m_ProgramPacket;
    /* 0x2934 */ GPU_CONTROLPACKET m_ControlPacket;
    /* 0x2964 */ GPU_TESSELLATORPACKET m_TessellatorPacket;
    /* 0x29b8 */ GPU_MISCPACKET m_MiscPacket;
    /* 0x2a50 */ GPU_POINTPACKET m_PointPacket;
} D3DDevice;

#pragma endregion
#pragma region D3DResource

typedef struct D3DResource { /* Size=0x18 */
    /* 0x0000 */ UINT Common;
    /* 0x0004 */ UINT ReferenceCount;
    /* 0x0008 */ UINT Fence;
    /* 0x000c */ UINT ReadFence;
    /* 0x0010 */ UINT Identifier;
    /* 0x0014 */ UINT BaseFlush;
} D3DResource;

// D3DResource methods
DWORD D3DResource_AddRef(D3DResource *self);
VOID D3DResource_BlockUntilNotBusy(D3DResource *self);
VOID D3DResource_GetDevice(D3DResource *self, D3DDevice **);
D3DRESOURCETYPE D3DResource_GetType(D3DResource *self);
BOOL D3DResource_IsBusy(D3DResource *self);
BOOL D3DResource_IsSet(D3DResource *self, D3DDevice *);
DWORD D3DResource_Release(D3DResource *self);

#pragma endregion
#pragma region D3DIndexBuffer

struct D3DIndexBuffer;

VOID D3DIndexBuffer_GetDesc(D3DIndexBuffer *self, D3DINDEXBUFFER_DESC *pDesc);
HANDLE
D3DIndexBuffer_Lock(D3DIndexBuffer *self, UINT OffsetToLock, UINT SizeToLock, DWORD Flags);
VOID D3DIndexBuffer_Unlock(D3DIndexBuffer *self);

typedef struct D3DIndexBuffer : public D3DResource { /* Size=0x20 */
    /* 0x0018 */ UINT Address;
    /* 0x001c */ UINT Size;

    HANDLE Lock(UINT offset, UINT size, DWORD flags) {
        return D3DIndexBuffer_Lock(this, offset, size, flags);
    }
    VOID Unlock() { D3DIndexBuffer_Unlock(this); }
} D3DIndexBuffer;

#pragma endregion
#pragma region D3DVertexBuffer

struct D3DVertexBuffer;

VOID D3DVertexBuffer_GetDesc(D3DVertexBuffer *self, D3DVERTEXBUFFER_DESC *pDesc);
HANDLE D3DVertexBuffer_Lock(
    D3DVertexBuffer *self, UINT OffsetToLock, UINT SizeToLock, DWORD Flags
);
VOID D3DVertexBuffer_Unlock(D3DVertexBuffer *self);

typedef struct D3DVertexBuffer : public D3DResource { /* Size=0x20 */
    /* 0x0018 */ GPUVERTEX_FETCH_CONSTANT Format;

    HANDLE Lock(UINT offset, UINT size, DWORD flags) {
        return D3DVertexBuffer_Lock(this, offset, size, flags);
    }
    VOID Unlock() { D3DVertexBuffer_Unlock(this); }
} D3DVertexBuffer;

D3DVertexBuffer *D3DDevice_CreateVertexBuffer(UINT Length, DWORD Usage, D3DPOOL Pool);

D3DIndexBuffer *
D3DDevice_CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool);

HRESULT D3DDevice_SetStreamSource(
    D3DDevice *,
    UINT StreamNumber,
    D3DVertexBuffer *pStreamData,
    UINT OffsetInBytes,
    UINT Stride,
    UINT64 unk_r8
);

#pragma endregion
#pragma region D3DSurface

struct D3DSurface : public D3DResource { /* Size=0x30 */
    union {
        struct {
            /* 0x0018 */ GPU_SURFACEINFO SurfaceInfo;
            union {
                /* 0x001c */ GPU_DEPTHINFO DepthInfo;
                /* 0x001c */ GPU_COLORINFO ColorInfo;
            };
            /* 0x0020 */ GPU_HICONTROL HiControl;
            /* 0x0024 */ UINT Height : 15; /* BitPos=3 */
            /* 0x0024 */ UINT Width : 14; /* BitPos=18 */
            /* 0x0028 */ D3DFORMAT Format;
            /* 0x002c */ UINT Size;
        };
        struct {
            /* 0x0018 */ struct D3DBaseTexture *Parent;
            /* 0x001c */ UINT MipLevel : 4; /* BitPos=28 */
        };
    };
};

#pragma endregion
#pragma region D3DTexture

struct D3DBaseTexture : public D3DResource { /* Size=0x34 */
    /* 0x0018 */ DWORD MipFlush;
    /* 0x001c */ GPUTEXTURE_FETCH_CONSTANT Format;
};

DWORD D3DBaseTexture_GetLevelCount(D3DBaseTexture *self);

struct D3DTexture : public D3DBaseTexture { /* Size=0x34 */
    /* 0x0000: fields for D3DBaseTexture */
};

VOID D3DTexture_GetLevelDesc(D3DTexture *, DWORD, D3DSURFACE_DESC *);
D3DSurface *D3DTexture_GetSurfaceLevel(D3DTexture *, UINT);
VOID D3DTexture_LockRect(D3DTexture *, UINT, D3DLOCKED_RECT *, const tagRECT *, UINT);
VOID D3DTexture_UnlockRect(D3DTexture *, UINT level);

struct D3DArrayTexture : public D3DBaseTexture { /* Size=0x34 */
    /* 0x0000: fields for D3DBaseTexture */
};

struct D3DCubeTexture : public D3DBaseTexture { /* Size=0x34 */
    /* 0x0000: fields for D3DBaseTexture */
};

struct D3DLineTexture : public D3DBaseTexture { /* Size=0x34 */
    /* 0x0000: fields for D3DBaseTexture */
};

struct D3DVolumeTexture : public D3DBaseTexture { /* Size=0x34 */
    /* 0x0000: fields for D3DBaseTexture */
};

#pragma endregion
#pragma region D3DVolume

struct D3DVolume : public D3DResource { /* Size=0x20 */
    /* 0x0018 */ D3DBaseTexture *Parent;
    /* 0x001c */ UINT ArrayIndex : 6; /* BitPos=22 */
    /* 0x001c */ UINT MipLevel : 4; /* BitPos=28 */
};

void D3DVolume_GetDesc(D3DVolume *, D3DVOLUME_DESC *);
void D3DVolume_LockBox(D3DVolume *, D3DLOCKED_BOX *, D3DBOX *, UINT);
void D3DVolume_UnlockBox(D3DVolume *);

#pragma endregion
#pragma region RenderState

uint D3DDevice_GetRenderState_BlendOp(D3DDevice *);
uint D3DDevice_GetRenderState_SrcBlend(D3DDevice *);
uint D3DDevice_GetRenderState_DestBlend(D3DDevice *);
uint D3DDevice_GetRenderState_SrcBlendAlpha(D3DDevice *);
uint D3DDevice_GetRenderState_DestBlendAlpha(D3DDevice *);

void D3DDevice_SetRenderState_AlphaBlendEnable(D3DDevice *, uint);
void D3DDevice_SetRenderState_AlphaFunc(D3DDevice *, D3DCMPFUNC);
void D3DDevice_SetRenderState_AlphaRef(D3DDevice *, DWORD);
void D3DDevice_SetRenderState_AlphaTestEnable(D3DDevice *, uint);
void D3DDevice_SetRenderState_BlendOp(D3DDevice *, uint);
void D3DDevice_SetRenderState_SrcBlend(D3DDevice *, uint);
void D3DDevice_SetRenderState_DestBlend(D3DDevice *, uint);
void D3DDevice_SetRenderState_SrcBlendAlpha(D3DDevice *, uint);
void D3DDevice_SetRenderState_DestBlendAlpha(D3DDevice *, uint);
void D3DDevice_SetRenderState_ColorWriteEnable(D3DDevice *, uint);
void D3DDevice_SetRenderState_FillMode(D3DDevice *, uint);
void D3DDevice_SetRenderState_CullMode(D3DDevice *, uint);
void D3DDevice_SetRenderState_StencilEnable(D3DDevice *, uint);
void D3DDevice_SetRenderState_StencilFail(D3DDevice *, uint);
void D3DDevice_SetRenderState_StencilZFail(D3DDevice *, uint);
void D3DDevice_SetRenderState_StencilPass(D3DDevice *, uint);
void D3DDevice_SetRenderState_StencilFunc(D3DDevice *, D3DCMPFUNC);
void D3DDevice_SetRenderState_StencilRef(D3DDevice *, DWORD);
void D3DDevice_SetRenderState_ZEnable(D3DDevice *, DWORD);
void D3DDevice_SetRenderState_ZFunc(D3DDevice *, D3DCMPFUNC);
void D3DDevice_SetRenderState_ZWriteEnable(D3DDevice *, DWORD);

#pragma endregion
#pragma region Misc

void D3DDevice_SetTexture(D3DDevice *, UINT, D3DBaseTexture *, UINT64);
D3DSurface *D3DDevice_GetRenderTarget(D3DDevice *, UINT);

#ifdef __cplusplus
}
#endif
