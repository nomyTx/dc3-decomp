#pragma once
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "utl/MemMgr.h"

class RndParticleSysAnim : public RndAnimatable {
public:
    // Hmx::Object
    virtual ~RndParticleSysAnim();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(ParticleSysAnim);
    OBJ_SET_TYPE(ParticleSysAnim);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void Print();
    // RndAnimatable
    virtual void SetFrame(float, float);
    virtual float EndFrame();
    virtual Hmx::Object *AnimTarget();
    virtual void SetKey(float);

    OBJ_MEM_OVERLOAD(0x18)
    NEW_OBJ(RndParticleSysAnim)
    static void Init() { REGISTER_OBJ_FACTORY(RndParticleSysAnim) }

protected:
    RndParticleSysAnim();
};
