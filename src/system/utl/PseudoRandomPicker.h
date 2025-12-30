#pragma once
#include <list>
#include <vector>

template <class T>
class PseudoRandomPicker {
public:
    PseudoRandomPicker() : unk8(1), unkc(0.5), unk10(0) {}
    ~PseudoRandomPicker() {}

    void AddItems(const std::vector<T> &itemVec) {
        for (int i = 0; i < itemVec.size(); i++) {
            T cur = itemVec[i];
            unk0.push_back(cur);
        }
    }

    void Randomize();

    const T GetItem(int);
    const T GetNext();

private:
    std::list<T> unk0; // 0x0 - items?
    int unk8;
    float unkc;
    int unk10;
};
