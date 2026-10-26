#pragma once

namespace logic {
inline int interface_language(int code) { return code == 2 ? 1 : (code == 3 ? 2 : 0); }
inline bool meets_margin(int party_level, int enemy_level, int margin) {
    return party_level > 0 && enemy_level > 0 && margin >= 0 && margin <= 99
        && party_level - enemy_level >= margin;
}
inline bool protected_group(const unsigned char* group) {
    return group[0x14] || group[0x15] || group[0x16] || group[0x17]
        || *reinterpret_cast<const int*>(group + 0x1c) != 0
        || *reinterpret_cast<const int*>(group + 0x58) != 0
        || *reinterpret_cast<const int*>(group + 0x60) != 0
        || *reinterpret_cast<const int*>(group + 0x68) != 0;
}
inline int max(int a, int b) { return a > b ? a : b; }
inline int min(int a, int b) { return a < b ? a : b; }
inline int accumulated_drop(int current, int added, int available) {
    return min(99, min(current + added, available));
}
inline int drop_weight(int total, int positive, int rate, bool item, int chance) {
    if (!item) return rate;
    float reciprocal = 1.0f / static_cast<float>(positive);
    int limit = static_cast<int>(static_cast<float>(total) * static_cast<float>(rate) * reciprocal + 0.5f);
    float factor = static_cast<float>(chance) * 0.01f;
    int adjusted = static_cast<int>(static_cast<float>(rate) * factor + 0.5f);
    return min(adjusted, limit);
}
inline int training_reward(int base, int basis_points) {
    long long product = static_cast<long long>(base) * basis_points;
    if (base < 0 || basis_points < 0 || product > 2147483647) return -1;
    // Native training rates use ten-thousandths, unlike ordinary reward bonuses.
    float value = static_cast<float>(product) * 0.0001f;
    int result = static_cast<int>(value);
    return result + (static_cast<float>(result) < value);
}
}
