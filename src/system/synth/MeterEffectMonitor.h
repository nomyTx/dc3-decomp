#pragma once
#include "FxSendMeterEffect.h"
#include "rndobj/Poll.h"

/** "Monitor for FxMeterEffect to feed back channel data" */
class MeterEffectMonitor : public RndPollable {
public:
    // Hmx::Object
    virtual ~MeterEffectMonitor();
    OBJ_CLASSNAME(MeterEffectMonitor);
    OBJ_SET_TYPE(MeterEffectMonitor);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndPollable
    virtual void Poll();

protected:
    MeterEffectMonitor();

    /** "FxSendMeterEffect for this object to monitor" */
    ObjPtr<FxSendMeterEffect> mMeterEffect; // 0x8
    float unk1c; // 0x1c
    float unk20; // 0x20
};
