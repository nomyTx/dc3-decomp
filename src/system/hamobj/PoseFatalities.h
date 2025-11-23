#pragma once
#include "obj/Object.h"
#include "utl/MemMgr.h"

class PoseFatalities : public Hmx::Object {
public:
    // Hmx::Object
    PoseFatalities();
    virtual ~PoseFatalities();
    OBJ_CLASSNAME(PoseFatalities);
    OBJ_SET_TYPE(PoseFatalities);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    NEW_OBJ(PoseFatalities)

    void DrawDebug();
    void Enter();
    bool InFatality(int) const;
    void ActivateFatal(int);
};
