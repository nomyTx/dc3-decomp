#include "gesture/DepthBuffer3D.h"
#include "obj/Object.h"
#include "rnddx9/Rnd.h"
#include "rndobj/Draw.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/Tex.h"
#include "rndobj/Trans.h"

LargeQuadRenderData DepthBuffer3D::mQuad;

DepthBuffer3D::DepthBuffer3D()
    : mDrawSheet(0), mDrawPlayer1(1), mDrawPlayer2(1), mDrawNonPlayers(1),
      mDebugLayout(0), mNobodyColor(0, 0, 0, 0), mPlayerPalette(this), unk12c(this),
      unk140(1), mPlayerPaletteOffset(0), mPlayerPaletteScale(1), mMinimalMat(this),
      mMesh(this), mStretchNearCamera(1), mOpacity(1), unk17c(0), unk180(0),
      unk184(0xfffffc19), unk188(1), unk18c(this), mTile(1.5, 1.5), mScaleVoxel(1),
      mScaleVoxelGap(1), mFishEyeX(0), mFishEyeY(0), unk1d8(this), unk1ec(this),
      unk20c(80, 4, 4), unk220(40, 4, 4), unk234(60, 3, 3), unk248(30, 3, 3),
      unk25c(2048, 204.8f, 204.8f), unk270(4096, 204.8f, 204.8f), mMaxZoom(1),
      mMaxDepthZoom(1), unk28c(0) {}

DepthBuffer3D::~DepthBuffer3D() {}

BEGIN_HANDLERS(DepthBuffer3D)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(DepthBuffer3D)
    SYNC_PROP(nobody_color, mNobodyColor)
    SYNC_PROP(nobody_alpha, mNobodyColor.alpha)
    SYNC_PROP_SET(
        player_palette, mPlayerPalette.Ptr(), SetPlayerPalette(_val.Obj<RndTex>())
    )
    SYNC_PROP(player_palette_offset, mPlayerPaletteOffset)
    SYNC_PROP(player_palette_scale, mPlayerPaletteScale)
    SYNC_PROP(minimal_mat, mMinimalMat)
    SYNC_PROP(draw_sheet, mDrawSheet)
    SYNC_PROP(mesh, mMesh)
    SYNC_PROP(stretch_near_camera, mStretchNearCamera)
    SYNC_PROP(opacity, mOpacity)
    SYNC_PROP(draw_player_1, mDrawPlayer1)
    SYNC_PROP(draw_player_2, mDrawPlayer2)
    SYNC_PROP(draw_non_players, mDrawNonPlayers)
    SYNC_PROP(tile_x, mTile.x)
    SYNC_PROP(tile_y, mTile.y)
    SYNC_PROP(scale_voxel, mScaleVoxel)
    SYNC_PROP(scale_voxelgap, mScaleVoxelGap)
    SYNC_PROP(fisheye_x, mFishEyeX)
    SYNC_PROP(fisheye_y, mFishEyeY)
    SYNC_PROP(max_zoom, mMaxZoom)
    SYNC_PROP(max_depth_zoom, mMaxDepthZoom)
    SYNC_PROP(debug_layout, mDebugLayout)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void DepthBuffer3D::Init() {
    REGISTER_OBJ_FACTORY(DepthBuffer3D);
    TheNgRnd.CreateLargeQuad(0x140, 0xF0, mQuad);
}
