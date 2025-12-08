#pragma once
#include "rndobj/ShaderOptions.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

class RndShaderBuffer {
public:
    virtual ~RndShaderBuffer() {}
    virtual void *Storage() = 0;
    virtual unsigned int Size() const = 0;

    MEM_OVERLOAD(RndShaderBuffer, 0x11);
};

class RndShaderProgram {
public:
    RndShaderProgram() {}
    virtual ~RndShaderProgram();
    virtual void Select(bool) = 0;
    virtual void Copy(const RndShaderProgram &) = 0;
    virtual void EstimatedCost(float &, float &) = 0;
    virtual RndShaderBuffer *NewBuffer(unsigned int) = 0;
    virtual bool
    Compile(ShaderType, const ShaderOptions &, RndShaderBuffer *&, RndShaderBuffer *&) = 0;
    virtual void CreateVertexShader(RndShaderBuffer &) = 0;
    virtual void CreatePixelShader(RndShaderBuffer &, ShaderType) = 0;

    void LoadShaderBuffer(BinStream &, int, RndShaderBuffer *&);
    bool Cache(ShaderType, const ShaderOptions &, RndShaderBuffer *, RndShaderBuffer *);

    static unsigned long InitModTime();

    int unk4;
    u64 unk8;
    bool unk18;

protected:
    void CopyErrorShader(ShaderType, const ShaderOptions &);
    void SaveShaderBuffer(const char *, RndShaderBuffer &);
    void LoadShaderBuffer(const char *, RndShaderBuffer *&);
};
