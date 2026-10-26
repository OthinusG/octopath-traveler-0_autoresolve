#include "../native/logic.hpp"
#include <cassert>

int main() {
    assert(logic::interface_language(2) == 1);
    assert(logic::interface_language(3) == 2);
    const int other_languages[] = {-1, 0, 1, 4, 5, 99};
    for (int code : other_languages) assert(logic::interface_language(code) == 0);
    assert(logic::meets_margin(11, 10, 1));
    assert(!logic::meets_margin(10, 10, 1));
    assert(logic::meets_margin(10, 10, 0));
    assert(!logic::meets_margin(99, 0, 1));
    assert(!logic::meets_margin(0, 1, 0));
    assert(!logic::meets_margin(99, 1, -1));
    assert(!logic::meets_margin(99, 1, 100));

    alignas(8) unsigned char group[0x90]{};
    assert(!logic::protected_group(group));
    const int offsets[] = {0x14, 0x15, 0x16, 0x17, 0x1c, 0x58, 0x60, 0x68};
    for (int offset : offsets) {
        group[offset] = 1;
        assert(logic::protected_group(group));
        group[offset] = 0;
    }

    assert(logic::drop_weight(100, 30, 30, true, 100) == 30);
    assert(logic::drop_weight(100, 30, 30, true, 200) == 60);
    assert(logic::drop_weight(100, 30, 30, true, 1000) == 100);
    assert(logic::drop_weight(100, 30, 70, false, 1000) == 70);
    assert(logic::drop_weight(64000000, 1, 1000000, false, 10000) == 1000000);
    assert(logic::drop_weight(1000, 10, 1, true, 150) == 2);
    assert(logic::accumulated_drop(1, 2, 999) == 3);
    assert(logic::accumulated_drop(98, 8, 999) == 99);
    assert(logic::accumulated_drop(1, 8, 5) == 5);
    assert(logic::accumulated_drop(0, 8, 0) == 0);
    assert(logic::training_reward(1, 10) == 1);
    assert(logic::training_reward(10, 1000) == 1);
    assert(logic::training_reward(11, 1000) == 2);
    assert(logic::training_reward(59, 7000) == 42);
    assert(logic::training_reward(40, 8000) == 32);
    assert(logic::training_reward(100, 0) == 0);
    assert(logic::training_reward(-1, 50) == -1);
    assert(logic::training_reward(99999999, 10000) == -1);
}
