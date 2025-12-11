#pragma once
#include "meta/FixedSizeSaveable.h"
#include "obj/Data.h"

enum SkillsAward {
};

class SkillsAwardList : public FixedSizeSaveable {
public:
    class Key {
    public:
    };
    SkillsAwardList();
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    bool HasFailure() const;
    SkillsAward GetAward(DataArray *);
    void SetAward(DataArray *, SkillsAward);
    void Clear();

protected:
    std::map<Key, SkillsAward> unk8;
    bool unk20;
};
