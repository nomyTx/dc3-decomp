#pragma once
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

class RndGenerator : public RndAnimatable, public RndTransformable, public RndDrawable {
public:
    // Hmx::Object
    virtual ~RndGenerator();
    OBJ_CLASSNAME(Mesh);
    OBJ_SET_TYPE(Mesh);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void Print();
    // RndAnimatable
    virtual void SetFrame(float, float);
    virtual float StartFrame();
    virtual float EndFrame();
    virtual void ListAnimChildren(std::list<RndAnimatable *> &) const;
    // RndDrawable
    virtual void UpdateSphere();
    virtual bool MakeWorldSphere(Sphere &, bool);
    virtual void DrawShowing();
    virtual void ListDrawChildren(std::list<RndDrawable *> &);
    // RndHighlightable
    virtual void Highlight() { RndDrawable::Highlight(); }

    OBJ_MEM_OVERLOAD(0x1F)
    NEW_OBJ(RndGenerator)
    static void Init() { REGISTER_OBJ_FACTORY(RndGenerator) }

protected:
    RndGenerator();
};
