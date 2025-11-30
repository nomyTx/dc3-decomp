#pragma once
#include "obj/Object.h"
#include "rnddx9/Object.h"
#include "rndobj/Mesh.h"
#include "xdk/D3D9.h"

class DxMesh : public RndMesh, public DxObject {
public:
    // Hmx::Object
    virtual ~DxMesh();
    OBJ_CLASSNAME(Mesh)
    OBJ_SET_TYPE(Mesh)
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    // RndMesh
    virtual void DrawShowing();
    virtual void DrawFacesInRange(int, int);
    virtual int NumFaces() const;
    virtual int NumVerts() const;
    virtual void OnSync(int);

    D3DVertexBuffer *GetMultimeshFaces();

protected:
    DxMesh();
};
