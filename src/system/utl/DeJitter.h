#pragma once

class DeJitter {
public:
    DeJitter();
    void Reset();
    float NewMs(float, float &);

    static float sTimeScale;

private:
    float unk0[0x20]; // 0x0
    int unk80; // 0x80
    int unk84; // 0x84
    float unk88; // 0x88
    float unk8c; // 0x8c
};
