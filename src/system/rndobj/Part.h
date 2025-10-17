#pragma once
#include "math/Color.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

class RndParticle {
public:
    MEM_ARRAY_OVERLOAD(Particle, 0x1E);

    Hmx::Color col; // 0x0
    Hmx::Color colVel; // 0x10
    Vector4 pos; // 0x20
    Vector4 vel; // 0x30
    float deathFrame; // 0x40
    float birthFrame; // 0x44
    float size; // 0x48
    float sizeVel; // 0x4c
    float angle; // 0x50
    float swingArm; // 0x54
    RndParticle *prev; // 0x58
    RndParticle *next; // 0x5c
};

class RndFancyParticle : public RndParticle {
public:
    float growFrame; // 0x60
    float growVel; // 0x64
    float shrinkFrame; // 0x68
    float shrinkVel; // 0x6c
    Hmx::Color midcolVel; // 0x70
    float midcolFrame; // 0x80
    float beginGrow; // 0x84
    float midGrow; // 0x88
    float endGrow; // 0x8c
    Vector4 bubbleDir; // 0x90
    float bubbleFreq; // 0xa0
    float bubblePhase; // 0xa4
    float RPF; // 0xa8
    float swingArmVel; // 0xac
};

class ParticleCommonPool {
public:
    ParticleCommonPool()
        : mPoolParticles(nullptr), mPoolFreeParticles(nullptr), mNumActiveParticles(0),
          mHighWaterMark(0) {}
    void InitPool();
    RndParticle *AllocateParticle();
    int NumActiveParticles() const { return mNumActiveParticles; }
    int HighWaterMark() const { return mHighWaterMark; }
    MEM_OVERLOAD(ParticleCommonPool, 0x254)

private:
    RndParticle *mPoolParticles; // 0x0
    RndParticle *mPoolFreeParticles; // 0x4
    int mNumActiveParticles; // 0x8
    int mHighWaterMark; // 0xc
};

class PartOverride {
public:
    PartOverride();

    unsigned int mask; // 0x0
    float life; // 0x4
    float speed; // 0x8
    float size; // 0xc
    float deltaSize; // 0x10
    Hmx::Color startColor; // 0x14
    Hmx::Color midColor; // 0x24
    Hmx::Color endColor; // 0x34
    Vector2 pitch; // 0x44
    Vector2 yaw; // 0x4c
    RndMesh *mesh; // 0x54
    Box box; // 0x58
};

// Attractor
class Attractor {
public:
    Attractor(Hmx::Object *owner) : mAttractor(owner), mStrength(0) {}
    void Save(BinStream &) const;
    void Load(BinStreamRev &);

    ObjPtr<RndTransformable> mAttractor; // 0x0
    float mStrength; // 0x14
};

class RndParticleSys : public RndAnimatable,
                       public RndPollable,
                       public RndTransformable,
                       public RndDrawable {
public:
    enum Type {
        kBasic = 0,
        kFancy = 1
    };
    // Hmx::Object
    virtual ~RndParticleSys();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(ParticleSys);
    OBJ_SET_TYPE(ParticleSys);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndAnimatable
    virtual void SetFrame(float, float);
    virtual float EndFrame();
    // RndPollable
    virtual void Poll();
    virtual void Enter();
    // RndDrawable
    virtual void UpdateSphere();
    virtual bool MakeWorldSphere(Sphere &, bool);
    virtual void Mats(std::list<class RndMat *> &, bool);
    virtual void DrawShowing();
    // RndHighlightable
    virtual void Highlight() { RndDrawable::Highlight(); }

    virtual void SetPreserveParticles(bool b);
    virtual void SetPool(int, Type);
    virtual void SetPersistentPool(int, Type);

    OBJ_MEM_OVERLOAD(0x90)
    NEW_OBJ(RndParticleSys)
    static void Init() { REGISTER_OBJ_FACTORY(RndParticleSys) }

    void SetRelativeMotion(float, RndTransformable *);
    void SetSubSamples(int);
    void SetMesh(RndMesh *);
    void FreeAllParticles();

protected:
    RndParticleSys();

    DataNode OnSetStartColor(const DataArray *);
    DataNode OnSetStartColorInt(const DataArray *);
    DataNode OnSetEndColor(const DataArray *);
    DataNode OnSetEndColorInt(const DataArray *);
    DataNode OnSetEmitRate(const DataArray *);
    DataNode OnAddEmitRate(const DataArray *);
    DataNode OnSetBurstInterval(const DataArray *);
    DataNode OnSetBurstPeak(const DataArray *);
    DataNode OnSetBurstLength(const DataArray *);
    DataNode OnExplicitPart(const DataArray *);
    DataNode OnExplicitParts(const DataArray *);
    DataNode OnSetLife(const DataArray *);
    DataNode OnSetSpeed(const DataArray *);
    DataNode OnSetRotate(const DataArray *);
    DataNode OnSetSwingArm(const DataArray *);
    DataNode OnSetDrag(const DataArray *);
    DataNode OnSetAlignment(const DataArray *);
    DataNode OnSetStartSize(const DataArray *);
    DataNode OnSetMat(const DataArray *);
    DataNode OnSetPos(const DataArray *);
    DataNode OnActiveParticles(const DataArray *);

    Type mType; // 0x118
    int mMaxParticles; // 0x11c
    RndParticle *mPersistentParticles; // 0x120
    RndParticle *mFreeParticles; // 0x124
    RndParticle *mActiveParticles; // 0x128
    int mNumActive; // 0x12c
    float mEmitCount; // 0x130
    bool unk134;
    float unk138;
    int unk13c;
    bool unk140;
    float unk144;
    Vector2 mBubblePeriod; // 0x148
    Vector2 mBubbleSize; // 0x150
    Vector2 mLife; // 0x158
    Vector3 mBoxExtent1; // 0x160
    Vector3 mBoxExtent2; // 0x170
    Vector2 mSpeed; // 0x180
    Vector2 mPitch; // 0x188
    Vector2 mYaw; // 0x190
    Vector2 mEmitRate; // 0x198
    float unk1a0;
    float unk1a4;
    float unk1a8;
    float unk1ac;
    float unk1b0;
    float unk1b4;
    float unk1b8;
    float unk1bc;
    float unk1c0;
    float unk1c4;
    float unk1c8;
    float unk1cc;
    float unk1d0;
    float unk1d4;
    float unk1d8;
    float unk1dc;
    float unk1e0;
    float unk1e4;
    float unk1e8;
    float unk1ec;
    ObjPtr<RndMesh> unk1f0;
    ObjPtr<RndMat> unk204;
    bool unk218;
    int unk21c;
    Vector3 unk220[7];
    float unk290;
    float unk294;
    float unk298;
    float mRelativeMotion; // 0x29c
    ObjOwnerPtr<RndTransformable> unk2a0; // 0x2a0
    Vector3 unk2b4;
    ObjPtr<RndTransformable> unk2c4; // 0x2c4
    Vector3 unk2d8;
    float unk2e8;
    bool unk2ec;
    bool unk2ed;
    bool unk2ee;
    bool unk2ef;
    float unk2f0;
    float unk2f4;
    float unk2f8;
    bool unk2fc;
    float unk300;
    float unk304;
    float unk308;
    float unk30c;
    bool unk310;
    bool unk311;
    bool unk312;
    bool unk313;
    float unk314;
    float unk318;
    int unk31c;
    Transform unk320;
    float unk360;
    float unk364;
    float unk368;
    float unk36c;
    float unk370;
    float unk374;
    float unk378;
    float unk37c;
    float unk380;
    float unk384;
    float unk388;
    bool unk38c;
    float unk390;
    int unk394;
    int unk398;
    int unk39c;
    int unk3a0;
    float unk3a4;
    float unk3a8;
    float unk3ac;
    float unk3b0;
    float unk3b4;
    float unk3b8;
    float unk3bc;
    int unk3c0;
    float unk3c4;
    bool unk3c8;
    bool unk3c9;
    bool unk3ca;
    float unk3cc;
    int unk3d0;
    int unk3d4;
    int unk3d8;
    int unk3dc;
    float unk3e0;
    float unk3e4;
    ObjVector<Attractor> mAttractors; // 0x3e8
};

extern ParticleCommonPool *gParticlePool;
extern PartOverride gNoPartOverride;
void InitParticleSystem();
