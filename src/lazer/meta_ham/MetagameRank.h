#pragma once
#include "meta/FixedSizeSaveable.h"
#include "meta_ham/HamProfile.h"
#include "obj/Data.h"
#include "obj/Object.h"

struct DeferredPoints {
    int unk0;
    Symbol unk4;
};

class MetagameRank : public Hmx::Object, public FixedSizeSaveable {
public:
    MetagameRank(HamProfile *);
    // Hmx::Object
    virtual DataNode Handle(DataArray *, bool);

    void Clear();
    int GetRankInTier() const;
    int GetTier() const;
    int GetXPOfRank(int) const;
    bool HasNewRank() const;
    void AwardPoints(int, Symbol);
    void AwardPointsForTask(Symbol);
    Symbol GetRankTitle() const;

    DataNode GetNextDeferredPoints(DataArray *);

    static int SaveSize(int);
    static void Preinit();
    static void Init();

private:
    // FixedSizeSaveable
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    bool GetOneTimeTask(Symbol, DataArray **, int *);
    int ComputeRankNumber(bool);

protected:
    int unk34;
    bool unk38;
    char unk39[0x40];
    char unk79[0x40];
    HamProfile *mProfile; // 0xbc
    int unkc0; // 0xc0 - level
    float unkc4;
    bool unkc8;
    bool unkc9;
    bool unkca;
    std::list<DeferredPoints> unkcc;
};
