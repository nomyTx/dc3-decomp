#pragma once
#include "os/Debug.h"
#include <cstdlib>
#include <list>
#include <vector>

template <class T>
class PseudoRandomPicker {
public:
    PseudoRandomPicker() : unk8(1), unkc(0.5), unk10(0) {}
    ~PseudoRandomPicker() {}

    void AddItem(const T item) { unk0.push_back(item); }

    void AddItems(const std::vector<T> &itemVec) {
        for (int i = 0; i < itemVec.size(); i++) {
            T cur = itemVec[i];
            unk0.push_back(cur);
        }
    }

    void Randomize();

    int Size() const { return unk0.size(); }
    void Clear() { unk0.resize(0); }

    const T GetItem(int);
    const T GetNext() {
        MILO_ASSERT(Size() > 0, 0x52);
        int idx;
        switch (unk8) {
        case 0:
            idx = 0;
            break;
        case 2:
            idx = rand() % (unk0.size() % unk10);
            break;
        case 3: {
            int i1 = unk0.size();
            idx = rand() % i1;
            break;
        }
        default:
            int i1 = unkc * (float)unk0.size() + 1.01f;
            idx = rand() % i1;
            break;
        }
        unk10++;
        return GetItem(idx);
    }

private:
    std::list<T> unk0; // 0x0 - items?
    int unk8;
    float unkc;
    int unk10;
};
