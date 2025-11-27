#pragma once

// size 0x44
struct MemPointDelta {
    bool AnyGreaterThan(int) const;
    const char *ToString(int) const;

    static const char *HeaderString(const char *);

    int unk0;
    int unk4;
};
