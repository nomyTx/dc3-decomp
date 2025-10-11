#pragma once
#include "rndobj/Poll.h"

class RndPartLauncher : public RndPollable {
public:
    // Hmx::Object
    virtual ~RndPartLauncher();
    OBJ_CLASSNAME(PartLauncher);
    OBJ_SET_TYPE(PartLauncher);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndPollable
    virtual void Poll();

    OBJ_MEM_OVERLOAD(0x13)
    NEW_OBJ(RndPartLauncher)
    static void Init() { REGISTER_OBJ_FACTORY(RndPartLauncher) }
};
