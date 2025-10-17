#include "rndobj/Part.h"
#include "math/Geo.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "os/System.h"
#include "utl/BinStream.h"

PartOverride gNoPartOverride;
ParticleCommonPool *gParticlePool;

namespace {
    int ParticlePoolSize() {
        return SystemConfig("rnd", "particlesys", "global_limit")->Int(1);
    }

    DataNode PrintParticlePoolSize(DataArray *) {
        MILO_LOG("Particle Pool Size:\n");
        if (gParticlePool) {
            int size = ParticlePoolSize();
            MILO_LOG(
                "   %d particles can be allocated, %.1f KB.\n",
                size,
                (float)(size * 200 * 0.0009765625f)
            );
            MILO_LOG(
                "   %d particles active, %d is the high water mark.\n",
                gParticlePool->NumActiveParticles(),
                gParticlePool->HighWaterMark()
            );
            MILO_LOG(
                "   Adding 30%%, suggesting a particle global limit of %d (set in default.dta).\n",
                (int)(gParticlePool->HighWaterMark() * 1.3f)
            );
        }
        return 0;
    }
}

BinStream &operator<<(BinStream &bs, const RndParticle &p) {
    bs << p.pos << p.col << p.size;
    return bs;
}

BinStream &operator>>(BinStream &bs, RndParticle &p) {
    bs >> p.pos >> p.col >> p.size;
    return bs;
}

PartOverride::PartOverride()
    : mask(0), life(0), speed(0), deltaSize(0), startColor(0), midColor(0), endColor(0),
      pitch(0, 0), yaw(0, 0), mesh(0), box(Vector3(0, 0, 0), Vector3(0, 0, 0)) {}

void InitParticleSystem() {
    if (!gParticlePool) {
        gParticlePool = new ParticleCommonPool();
    }
    if (gParticlePool) {
        gParticlePool->InitPool();
    }
    DataRegisterFunc("print_particle_pool_size", PrintParticlePoolSize);
}

void ParticleCommonPool::InitPool() {
    int size = ParticlePoolSize();
    mPoolParticles = new RndFancyParticle[size];
    for (int i = 0; i < size - 1; i++) {
        mPoolParticles[i].prev = nullptr;
        mPoolParticles[i].next = &mPoolParticles[i + 1];
    }
    mPoolParticles[size - 1].prev = nullptr;
    mPoolParticles[size - 1].next = nullptr;
    mPoolFreeParticles = mPoolParticles;
}

RndParticle *ParticleCommonPool::AllocateParticle() {
    RndParticle *cur = mPoolFreeParticles;
    RndParticle *ret = nullptr;
    if (cur) {
        mPoolFreeParticles = mPoolFreeParticles->next;
        cur->prev = cur;
        mNumActiveParticles++;
        ret = cur;
        if (mNumActiveParticles > mHighWaterMark) {
            mHighWaterMark = mNumActiveParticles;
        }
    }
    return ret;
}

BEGIN_CUSTOM_PROPSYNC(Attractor)
    SYNC_PROP(attractor, o.mAttractor)
    SYNC_PROP(strength, o.mStrength)
END_CUSTOM_PROPSYNC

void Attractor::Save(BinStream &bs) const {
    bs << mAttractor;
    bs << mStrength;
}

void Attractor::Load(BinStreamRev &d) {
    d >> mAttractor;
    d >> mStrength;
}

RndParticleSys::RndParticleSys()
    : mType(kBasic), mMaxParticles(0), mPersistentParticles(nullptr),
      mFreeParticles(nullptr), mActiveParticles(nullptr), mNumActive(0), mEmitCount(0),
      unk134(0), unk138(0), unk13c(0), unk140(0), unk144(0), mBubblePeriod(10, 10),
      mBubbleSize(1, 1), mLife(100, 100), mBoxExtent1(0, 0, 0), mBoxExtent2(0, 0, 0),
      mSpeed(1, 1), mPitch(0, 0), mYaw(0, 0), mEmitRate(1, 1), unk1a0(gUnitsPerMeter / 4),
      unk1a4(gUnitsPerMeter / 4), unk1a8(0), unk1ac(0), unk1b0(1), unk1b4(1), unk1b8(1),
      unk1bc(1), unk1c0(1), unk1c4(1), unk1c8(1), unk1cc(1), unk1d0(1), unk1d4(1),
      unk1d8(1), unk1dc(1), unk1e0(1), unk1e4(1), unk1e8(1), unk1ec(1), unk1f0(this),
      unk204(this), unk218(0), unk2a0(this), unk2c4(this), unk2d8(1, 1, 1), unk2e8(1),
      unk2ec(0), unk2ed(0), unk2ee(0), unk2ef(0), unk2f0(1), unk2f4(1), unk2f8(1),
      unk2fc(1), unk300(1), unk304(1), unk308(1), unk30c(1), unk310(0), unk311(0),
      unk312(0), unk313(0), unk314(1), unk318(1), unk31c(0), unk360(0), unk364(1),
      unk368(0.5), unk36c(1), unk370(1), unk374(1), unk378(1), unk37c(1), unk380(1),
      unk384(1), unk388(1), unk38c(0), unk390(1), unk394(0), unk398(0), unk39c(0),
      unk3a0(0), unk3a4(1), unk3a8(15), unk3ac(35), unk3b0(4), unk3b4(8), unk3b8(20),
      unk3bc(30), unk3c0(0), unk3c4(0), unk3c8(0), unk3c9(1), unk3ca(0), unk3cc(0),
      unk3d0(1), unk3d4(1), unk3d8(1), unk3dc(0), unk3e0(1), unk3e4(1),
      mAttractors(this) {
    SetRelativeMotion(0, this);
    SetSubSamples(0);
}

BEGIN_HANDLERS(RndParticleSys)
    HANDLE_EXPR(hi_emit_rate, Max(mEmitRate.x, mEmitRate.y))
    HANDLE(set_start_color, OnSetStartColor)
    HANDLE(set_end_color, OnSetEndColor)
    HANDLE(set_start_color_int, OnSetStartColorInt)
    HANDLE(set_end_color_int, OnSetEndColorInt)
    HANDLE(set_emit_rate, OnSetEmitRate)
    HANDLE(set_burst_interval, OnSetBurstInterval)
    HANDLE(set_burst_peak, OnSetBurstPeak)
    HANDLE(set_burst_length, OnSetBurstLength)
    HANDLE(add_emit_rate, OnAddEmitRate)
    HANDLE(launch_part, OnExplicitPart)
    HANDLE(launch_parts, OnExplicitParts)
    HANDLE(set_life, OnSetLife)
    HANDLE(set_speed, OnSetSpeed)
    HANDLE(set_rotate, OnSetRotate)
    HANDLE(set_swing_arm, OnSetSwingArm)
    HANDLE(set_drag, OnSetDrag)
    HANDLE(set_alignment, OnSetAlignment)
    HANDLE(set_start_size, OnSetStartSize)
    HANDLE(set_mat, OnSetMat)
    HANDLE(set_pos, OnSetPos)
    HANDLE_ACTION(set_mesh, SetMesh(_msg->Obj<RndMesh>(2)))
    HANDLE(active_particles, OnActiveParticles)
    HANDLE_EXPR(max_particles, mMaxParticles)
    HANDLE_ACTION(
        set_relative_parent,
        SetRelativeMotion(mRelativeMotion, _msg->Obj<RndTransformable>(2))
    )
    HANDLE_ACTION(clear_all_particles, FreeAllParticles())
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
