#include "game_build.hpp"
#include "logic.hpp"

extern "C" int _fltused = 0;
extern "C" void* memset(void* target, int value, u64 count) {
    auto* out = static_cast<u8*>(target);
    for (u64 i = 0; i < count; ++i) out[i] = static_cast<u8>(value);
    return target;
}
extern "C" void* memcpy(void* target, const void* source, u64 count) {
    auto* out = static_cast<u8*>(target);
    auto* in = static_cast<const u8*>(source);
    for (u64 i = 0; i < count; ++i) out[i] = in[i];
    return target;
}

namespace {
u8* image;
Handle library;
bool installed, enabled, failed, ordinary_scope, chinese, traditional;
void* prepared_field;
void* prepared_choice;
#ifdef OT0_DEVELOPMENT
void* prepared_update;
bool snapshot_written;
int interval_token, preview_token;
#endif
int margin = 1;
WChar config_path[1024], log_path[1024];

template<class T> T native(u32 rva) { return reinterpret_cast<T>(image + rva); }
int get(u32 rva, int id) { return native<int(*)(int)>(rva)(id); }

void append(WChar*& out, const WChar* text) { while (*text) *out++ = *text++; *out = 0; }
void number(WChar*& out, int value) {
    WChar digits[16]; int count = 0;
    do { digits[count++] = static_cast<WChar>(L'0' + value % 10); value /= 10; } while (value && count < 15);
    while (count) *out++ = digits[--count];
    *out = 0;
}
void log(const char* message, int a = 0, int b = 0, int c = 0) {
    char line[256]; int count = 0;
    while (*message && count < 180) line[count++] = *message++;
    int values[3] = {a, b, c};
    for (int index = 0; index < 3; ++index) {
        int value = values[index];
        line[count++] = ' ';
        if (value < 0) { line[count++] = '-'; value = -value; }
        char digits[16]; int n = 0;
        do { digits[n++] = static_cast<char>('0' + value % 10); value /= 10; } while (value && n < 15);
        while (n) line[count++] = digits[--n];
    }
    line[count++] = '\r'; line[count++] = '\n';
    Handle file = CreateFileW(log_path, 4, 3, nullptr, 4, 0x80, nullptr);
    if (file != reinterpret_cast<Handle>(-1)) { u32 written; WriteFile(file, line, count, &written, nullptr); CloseHandle(file); }
}
void paths() {
    GetModuleFileNameW(library, config_path, 1000);
    int i = 0; while (config_path[i]) ++i;
    while (i && config_path[--i] != L'\\') {}
    while (i && config_path[--i] != L'\\') {}
    WChar* out = config_path + i + 1; append(out, L"config.ini");
    for (int j = 0; j <= i; ++j) log_path[j] = config_path[j];
    out = log_path + i + 1; append(out, L"OT0AutoResolve.log");
}
void settings() {
    enabled = GetPrivateProfileIntW(L"General", L"Enabled", 0, config_path) != 0;
    margin = static_cast<int>(GetPrivateProfileIntW(L"General", L"LevelMargin", 1, config_path));
    if (margin < 0 || margin > 99) { enabled = false; margin = 1; }
}
void update_language() {
    int language = logic::interface_language(native<int(*)(void*, u8)>(build::InterfaceLanguage)(nullptr, 0));
    chinese = language != 0;
    traditional = language == 2;
}

struct Text { u64 data; u32 flags; u32 padding; };
struct String { const WChar* data; int count; int capacity; };
void notice(const WChar* title, const WChar* message) {
    if (!GetPrivateProfileIntW(L"General", L"ShowUI", 1, config_path)) return;
    void* menu = native<void*(*)()>(build::Menu)();
    if (!menu) return;
    void* ui = field<void*>(menu, 0x2c8);
    if (!ui) return;
    Text heading{}, detail{};
    auto make = native<Text*(*)(Text*, const String&)>(build::TextFromString);
    auto destroy = native<void(*)(Text*)>(build::TextDestroy);
    int n = 0; while (title[n]) ++n;
    String heading_string{title, n + 1, n + 1}; make(&heading, heading_string);
    n = 0; while (message[n]) ++n;
    String detail_string{message, n + 1, n + 1}; make(&detail, detail_string);
    native<void(*)(void*, u8, const Text&, const Text&, u8, int, u8, int, u8)>(build::Notice)
        (ui, 0, heading, detail, 0, 0, 0, 0, 0);
    destroy(&detail); destroy(&heading);
}

struct Member { int id, exp, jp, hp, sp, level, gain_exp, gain_jp; };
struct Item { int id, count, before; };
struct Plan {
    Member members[40]; int member_count;
    Item items[6]; int item_count;
    int exp, jp, money, group, highest, danger;
};
using PartyGetter = const Array<int>&(*)(int, bool, bool, bool, bool, bool);
alignas(16) u8 rate_context[0x650];

bool plan_modifiers(void* manager, const int* ids, int count, Plan& plan, int& drop_rate, int& quantity_rate) {
    void* save = native<void*(*)(void*)>(build::Save)(manager);
    if (!save) return false;
    // Meals have turn-limited effects; retain native combat while a meal is active.
    if (field<int>(save, 0x108) > 0 && field<int>(save, 0x10c) > 0) return false;
    void* work = native<void*(*)(void*)>(build::WorkBattle)(save);
    if (!work) return false;
    int regular[4]{}, equipment[4]{};
    drop_rate = 100;
    for (int i = 0; i < count; ++i) {
        for (int equipped = 0; equipped < 2; ++equipped) {
            int values[4]{};
            native<bool(*)(int*, int*, int*, int*, int, bool)>(build::SupportRates)
                (&values[0], &values[1], &values[2], &values[3], ids[i], equipped != 0);
            for (int j = 0; j < 4; ++j) {
                if (values[j] < 0 || values[j] > 10000) return false;
                int& target = equipped ? equipment[j] : regular[j];
                target = logic::max(target, values[j]);
            }
            int chance = 0;
            native<bool(*)(int*, int, bool)>(build::DropRate)(&chance, ids[i], equipped != 0);
            if (chance < 0 || chance > 10000) return false;
            drop_rate += chance;
        }
    }
    int resident[3]{};
    native<void(*)(int*, int*, int*)>(build::ResidentRates)(&resident[0], &resident[1], &resident[2]);
    int limits[3] = {field<int>(work, 0x8c), field<int>(work, 0x398), field<int>(work, 0x90)};
    memset(rate_context, 0, sizeof(rate_context));
    const int kinds[3] = {100, 119, 101};
    for (int j = 0; j < 3; ++j) {
        int cap = limits[j] >= -1000 && limits[j] <= 1000 ? limits[j] : 50;
        if (cap < 0 || resident[j] < 0 || resident[j] > 10000) return false;
        int* rates = reinterpret_cast<int*>(rate_context + 0x5c8 + j * 24);
        rates[1] = logic::min(regular[j], cap);
        rates[2] = equipment[j];
        rates[3] = resident[j];
        rates[5] = native<int(*)(u8)>(build::BuiltBonus)(static_cast<u8>(kinds[j]));
        if (rates[5] < 0 || rates[5] > 10000) return false;
    }
    void* battle = native<void*(*)()>(build::BattleManager)();
    int previous_money_bonus = battle ? field<int>(battle, 0x618) : 0;
    // The native calculation only reads these six-rate arrays and records a UI-only money delta.
    native<void(*)(void*, int*, int*, int*)>(build::RatedResult)(rate_context, &plan.exp, &plan.jp, &plan.money);
    if (battle) field<int>(battle, 0x618) = previous_money_bonus;
    quantity_rate = regular[3] + native<int(*)(u8)>(build::BuiltBonus)(102);
    drop_rate += native<int(*)(u8)>(build::BuiltBonus)(161);
    if (plan.exp < 0 || plan.jp < 0 || plan.money < 0 || plan.exp > 99999999 || plan.jp > 99999999
        || plan.money > 99999999 || drop_rate < 0 || drop_rate > 10000 || quantity_rate < 0 || quantity_rate > 10000) return false;
    for (int i = 0; i < plan.member_count; ++i) {
        plan.members[i].gain_exp = plan.exp;
        plan.members[i].gain_jp = plan.jp;
    }
    Array<int> trainees{};
    native<void(*)(Array<int>*)>(build::TrainingPlayers)(&trainees);
    bool valid = trainees.valid(32);
    int training_ids[32], training_count = valid ? trainees.count : 0;
    for (int i = 0; i < training_count; ++i) training_ids[i] = trainees.data[i];
    if (trainees.data) native<void(*)(void*)>(build::Free)(trainees.data);
    if (!valid) return false;
    if (training_count) {
        int exp_rate = native<int(*)(u8, void*)>(build::PlacedResidentRate)(239, nullptr)
            + native<int(*)(u8)>(build::TrainingRate)(244);
        int jp_rate = native<int(*)(u8, void*)>(build::PlacedResidentRate)(240, nullptr)
            + native<int(*)(u8)>(build::TrainingRate)(245);
        if (exp_rate < 0 || exp_rate > 10000 || jp_rate < 0 || jp_rate > 10000) return false;
        int training_exp = logic::training_reward(plan.exp, exp_rate);
        int training_jp = logic::training_reward(plan.jp, jp_rate);
        if (training_exp < 0 || training_jp < 0) return false;
        for (int i = 0; i < training_count; ++i) {
            int id = training_ids[i];
            if (id <= 0) return false;
            for (int j = 0; j < count; ++j) if (ids[j] == id) return false;
            for (int j = 0; j < i; ++j) if (training_ids[j] == id) return false;
            plan.members[plan.member_count++] = {id, get(build::EXP, id), get(build::JP, id), get(build::HP, id),
                get(build::SP, id), get(build::Level, id), training_exp, training_jp};
        }
    }
    return true;
}

bool plan_drop(const void* enemy, int drop_rate, int quantity_rate, Plan& plan) {
    const auto& special = field<Array<int>>(enemy, 0xa8);
    if (!special.valid(32)) return false;
    for (int i = 0; i < special.count; ++i) if (special.data[i] > 0) return false;
    int drop_id = field<int>(enemy, 0x9c);
    if (!drop_id) return true;
    const void* table = native<const void*(*)(int)>(build::Drop)(drop_id);
    if (!table) return false;
    const auto& items = field<Array<int>>(table, 0x10);
    const auto& rates = field<Array<int>>(table, 0x20);
    const auto& fixed = field<Array<int>>(table, 0x30);
    const auto& random = field<Array<int>>(table, 0x40);
    const auto& quests = field<Array<int>>(table, 0x50);
    if (!items.valid(64) || !rates.valid(64) || !fixed.valid(64) || !random.valid(64) || !quests.valid(64)
        || items.count != rates.count || items.count != fixed.count || items.count != random.count) return false;
    for (int i = 0; i < quests.count; ++i) if (quests.data[i] > 0) return false;
    int total = 0, positive = 0;
    for (int i = 0; i < items.count; ++i) {
        if (rates.data[i] < 0 || rates.data[i] > 1000000 || fixed.data[i] < 0 || random.data[i] < 0
            || fixed.data[i] > 999 || random.data[i] > 999) return false;
        total += rates.data[i];
        if (items.data[i] > 0) positive += rates.data[i];
    }
    if (!total || !positive) return true;
    int draw = native<int(*)(int, int)>(build::Random)(total - 1, 0);
    int cumulative = 0;
    for (int i = 0; i < items.count; ++i) {
        cumulative += logic::drop_weight(total, positive, rates.data[i], items.data[i] > 0, drop_rate);
        if (draw >= logic::min(total, cumulative)) continue;
        int id = items.data[i];
        if (id <= 0) return true;
        int quantity = fixed.data[i] + native<int(*)(int, int)>(build::Random)(random.data[i], 0);
        quantity = static_cast<int>(static_cast<i64>(quantity) * (100 + quantity_rate) / 100);
        int index = 0;
        while (index < plan.item_count && plan.items[index].id != id) ++index;
        if (index == plan.item_count) {
            if (index >= 6) return false;
            int before = get(build::ItemCount, id), maximum = get(build::ItemMax, id);
            if (before < 0 || maximum < before || maximum > 99999999) return false;
            plan.items[plan.item_count++] = {id, 0, before};
        }
        Item& item = plan.items[index];
        item.count = logic::accumulated_drop(item.count, quantity, get(build::ItemMax, id) - item.before);
        return true;
    }
    return true;
}

bool prepare(void* manager, int group_id, Plan& plan, bool roll_drops) {
    plan = {};
    plan.group = group_id;
    const void* group = native<const void*(*)(int)>(build::EnemyGroup)(group_id);
    if (!group || logic::protected_group(static_cast<const u8*>(group))) return false;
    const auto& events = field<Array<int>>(group, 0x48);
    if (!events.valid(32)) return false;
    for (int i = 0; i < events.count; ++i) if (events.data[i] > 0) return false;
    const auto& enemies = field<Array<int>>(group, 0x38);
    if (!enemies.valid(6) || !enemies.count) return false;
    int ids[8], party_count;
    int active = native<int(*)()>(build::ActiveParty)();
    const auto& party = native<PartyGetter>(build::Party)(active, true, true, true, false, false);
    if (!party.valid(8) || !party.count) return false;
    party_count = party.count;
    for (int i = 0; i < party_count; ++i) ids[i] = party.data[i];
    for (int i = 0; i < party_count; ++i) {
        int id = ids[i];
        if (id <= 0) return false;
        for (int j = 0; j < i; ++j) if (ids[j] == id) return false;
        int level = get(build::Level, id);
        if (level < 1 || level > 99) return false;
        plan.highest = logic::max(plan.highest, level);
        int hp = get(build::HP, id);
        if (hp <= 0) continue;
        plan.members[plan.member_count++] = {id, get(build::EXP, id), get(build::JP, id), hp, get(build::SP, id), level, 0, 0};
    }
    if (!plan.member_count) return false;
    plan.danger = native<int(*)()>(build::Danger)();
    if (!logic::meets_margin(plan.highest, plan.danger, margin)) return false;
    for (int i = 0; i < enemies.count; ++i) {
        int id = enemies.data[i];
        if (!id) continue;
        if (id < 0) return false;
        const void* enemy = native<const void*(*)(int)>(build::Enemy)(id);
        if (!enemy || field<u8>(enemy, 0x88)
            || !logic::meets_margin(plan.highest, get(build::EnemyLevel, id), margin)) return false;
        int exp = field<int>(enemy, 0x8c), jp = field<int>(enemy, 0x98), money = field<int>(enemy, 0x90);
        if (exp < 0 || jp < 0 || money < 0 || exp > 1000000 || jp > 1000000 || money > 1000000) return false;
        plan.exp += exp; plan.jp += jp; plan.money += money;
    }
    int drop_rate, quantity_rate;
    if (!plan_modifiers(manager, ids, party_count, plan, drop_rate, quantity_rate)) return false;
    for (int i = 0; i < plan.member_count; ++i) {
        const Member& member = plan.members[i];
        if (member.exp < 0 || member.exp > 999999999 || member.jp < 0 || member.jp > 999999
            || member.sp < 0 || member.level < 1 || member.level > 99
            || member.gain_exp < 0 || member.gain_exp > 99999999 || member.gain_jp < 0 || member.gain_jp > 99999999) return false;
    }
    for (int i = 0; i < enemies.count; ++i) {
        int id = enemies.data[i];
        if (roll_drops && id > 0 && !plan_drop(native<const void*(*)(int)>(build::Enemy)(id), drop_rate, quantity_rate, plan)) return false;
    }
    return plan.exp || plan.jp || plan.money;
}

void settle(Plan& plan) {
    update_language();
    bool ok = true;
    for (int i = 0; i < plan.member_count; ++i) {
        const Member& member = plan.members[i];
        int levels = 0;
        if (member.gain_exp > 0) levels = native<int(*)(int, int, bool)>(build::AddEXP)(member.id, member.gain_exp, false);
        if (levels > 0) {
            bool hp = native<bool(*)(int, int)>(build::SetHP)(member.id, get(build::MaxHP, member.id));
            bool sp = native<bool(*)(int, int)>(build::SetSP)(member.id, get(build::MaxSP, member.id));
            ok = hp && sp && ok;
        }
        if (member.gain_jp > 0) native<int(*)(int, int)>(build::AddJP)(member.id, member.gain_jp);
        int exp_after = get(build::EXP, member.id), jp_after = get(build::JP, member.id);
        if (levels < 0 || exp_after < member.exp || jp_after < member.jp) ok = false;
        log("Member EXP delta JP delta", member.id, exp_after - member.exp, jp_after - member.jp);
        if (levels > 0) log("Level-up HP SP", get(build::Level, member.id), get(build::HP, member.id), get(build::SP, member.id));
    }
    int money_before = 0, money_after = 0;
    ok = native<bool(*)(int, int*, int*)>(build::AddMoney)(plan.money, &money_before, &money_after) && ok;
    log("Leaves before after gain", money_before, money_after, plan.money);
    int item_total = 0;
    for (int i = 0; i < plan.item_count; ++i) {
        const Item& item = plan.items[i];
        if (!item.count) continue;
        int before = 0, after = 0;
        native<void(*)(int, int, int*, int*, bool)>(build::AddItem)(item.id, item.count, &before, &after, false);
        if (before != item.before || after - before != item.count) ok = false;
        item_total += logic::max(0, after - before);
        log("Item before after", item.id, before, after);
    }
    if (!ok) {
        failed = true; enabled = false;
        WritePrivateProfileStringW(L"General", L"Enabled", L"0", config_path);
        log("Settlement verification failed; disabled without retry");
        notice(chinese ? (traditional ? L"自動結算" : L"自动结算") : L"Auto Resolve",
            chinese ? (traditional ? L"獎勵更新失敗，已停用自動結算，請查看日誌。" : L"奖励更新失败，已停用自动结算，请查看日志。")
                : L"Reward update failed. Auto Resolve disabled; see log.");
        return;
    }
    WChar message[256]{}; WChar* out = message;
    append(out, L"EXP +"); number(out, plan.exp);
    append(out, L"   JP +"); number(out, plan.jp);
    append(out, chinese ? (traditional ? L"   金錢 +" : L"   金钱 +") : L"   Leaves +"); number(out, plan.money);
    append(out, chinese ? L"   物品 +" : L"   Items +"); number(out, item_total);
    notice(chinese ? (traditional ? L"自動結算" : L"自动结算") : L"Auto Resolve", message);
    log("Resolved field group EXP JP", plan.group, plan.exp, plan.jp);
}

using Standby = bool(*)(void*, bool, int, bool, int, int, int, int, bool, int, bool);
bool field_standby(void* manager, bool event, int a, bool b, int c, int d, int e, int f, bool g, int h, bool i) {
    bool previous = ordinary_scope;
    ordinary_scope = !event && a == -1 && !b && c == 0 && d == 0 && e == 0 && f == 0 && g && h == 0 && !i;
    bool result = native<Standby>(build::Standby)(manager, event, a, b, c, d, e, f, g, h, i);
    ordinary_scope = previous;
    return result;
}
bool choice(void* manager, int* group, const void* volume) {
    bool result = native<bool(*)(void*, int*, const void*)>(build::Choice)(manager, group, volume);
    if (!result || !ordinary_scope || !group || failed) return result;
    settings();
    bool diagnose = false;
#ifdef OT0_DEVELOPMENT
    diagnose = GetPrivateProfileIntW(L"Development", L"Diagnostics", 0, config_path) != 0;
#endif
    if (!enabled && !diagnose) return result;
    Plan plan;
    bool eligible = prepare(manager, *group, plan, enabled);
    log(eligible ? "Eligible field choice" : "Normal field choice", *group, plan.highest, plan.danger);
    if (eligible) log("Planned EXP JP leaves", plan.exp, plan.jp, plan.money);
    if (enabled && eligible) { settle(plan); return false; }
    return result;
}

#ifdef OT0_DEVELOPMENT
bool update_probe(void* manager) {
    int before = field<int>(manager, 0x394);
    bool result = native<bool(*)(void*)>(build::Update)(manager);
    if (!snapshot_written) {
        snapshot_written = true;
        log("Field danger step threshold", native<int(*)()>(build::Danger)(), field<int>(manager, 0x394), field<int>(manager, 0x390));
        const auto& party = native<PartyGetter>(build::Party)(native<int(*)()>(build::ActiveParty)(), true, true, true, false, false);
        if (party.valid(8)) {
            int ids[8], count = party.count;
            for (int i = 0; i < count; ++i) ids[i] = party.data[i];
            for (int i = 0; i < count; ++i) {
                log("Party id level HP", ids[i], get(build::Level, ids[i]), get(build::HP, ids[i]));
                log("Party EXP JP SP", get(build::EXP, ids[i]), get(build::JP, ids[i]), get(build::SP, ids[i]));
            }
        }
        void* save = native<void*(*)(void*)>(build::Save)(manager);
        if (save) log("Meal id remaining UI language", field<int>(save, 0x108), field<int>(save, 0x10c), native<int(*)(void*, u8)>(0x4778de0)(manager, 0));
    }
    if (result && field<int>(manager, 0x394) > before) {
        int requested_interval = static_cast<int>(GetPrivateProfileIntW(L"Development", L"ShortInterval", 0, config_path));
        if (requested_interval > 0 && requested_interval != interval_token) {
            int threshold = field<int>(manager, 0x390);
            if (field<u8>(manager, 0x3bc)) threshold = static_cast<int>(threshold * field<float>(manager, 0x398));
            if (threshold > 0 && threshold <= 100000) {
                field<int>(manager, 0x394) = threshold;
                interval_token = requested_interval;
                log("One development interval requested", interval_token);
            }
        }
        int requested_preview = static_cast<int>(GetPrivateProfileIntW(L"Development", L"Preview", 0, config_path));
        if (requested_preview > 0 && requested_preview != preview_token) {
            preview_token = requested_preview;
            chinese = requested_preview == 2 || requested_preview == 3;
            traditional = requested_preview == 3;
            notice(chinese ? L"自动结算（预览）" : L"Auto Resolve (preview)",
                chinese ? L"通知预览：EXP +10  JP +2  金钱 +20" : L"Notification preview: EXP +10  JP +2  Leaves +20");
            log("Notification preview sent; no rewards granted");
        }
    }
    return result;
}
#endif

bool matches_call(u32 site, u32 target) {
    return image[site] == 0xe8 && site + 5 + field<int>(image, site + 1) == target;
}
void* thunk(u32 site, void* target) {
    u64 origin = reinterpret_cast<u64>(image + site) & ~u64(0xffff);
    for (u64 distance = 0x10000; distance < 0x70000000; distance += 0x10000) {
        for (int direction = 0; direction < 2; ++direction) {
            u64 address = direction ? origin - distance : origin + distance;
            auto* block = static_cast<u8*>(VirtualAlloc(reinterpret_cast<void*>(address), 4096, 0x3000, 0x04));
            if (!block) continue;
            block[0] = 0xff; block[1] = 0x25;
            field<u32>(block, 2) = 0; field<void*>(block, 6) = target;
            u32 old;
            if (!VirtualProtect(block, 4096, 0x20, &old)) return nullptr;
            FlushInstructionCache(GetCurrentProcess(), block, 14);
            return block;
        }
    }
    return nullptr;
}
bool patch_call(u32 site, void* target) {
    i64 displacement = reinterpret_cast<i64>(target) - reinterpret_cast<i64>(image + site + 5);
    if (displacement < -2147483647LL || displacement > 2147483647LL) return false;
    u32 old;
    if (!VirtualProtect(image + site, 5, 0x40, &old)) return false;
    field<int>(image, site + 1) = static_cast<int>(displacement);
    u32 unused; VirtualProtect(image + site, 5, old, &unused);
    FlushInstructionCache(GetCurrentProcess(), image + site, 5);
    return true;
}
bool prepare_hooks() {
    paths(); settings(); image = static_cast<u8*>(GetModuleHandleW(nullptr));
    if (!image || field<u16>(image, 0) != 0x5a4d) return false;
    u32 pe = field<u32>(image, 0x3c);
    if (pe > 4096 || field<u32>(image, pe) != 0x4550
        || field<u32>(image, pe + 8) != build::Timestamp || field<u32>(image, pe + 80) != build::ImageSize
        || !matches_call(build::FieldCall, build::Standby) || !matches_call(build::ChoiceCall, build::Choice)
#ifdef OT0_DEVELOPMENT
        || !matches_call(build::UpdateCall, build::Update)
#endif
        ) {
        log("Unsupported executable; no hooks installed"); return false;
    }
    for (const auto& signature : build::Functions) {
        for (int i = 0; i < 8; ++i) if (image[signature.rva + i] != signature.bytes[i]) {
            log("Native function fingerprint mismatch", static_cast<int>(signature.rva));
            return false;
        }
    }
    // Lua reload may release its DLL handle while native callsites still point here.
    Handle resident;
    if (!GetModuleHandleExW(5, reinterpret_cast<const WChar*>(library), &resident)) {
        log("Could not retain native module; no hooks installed"); return false;
    }
    prepared_field = thunk(build::FieldCall, reinterpret_cast<void*>(&field_standby));
    prepared_choice = thunk(build::ChoiceCall, reinterpret_cast<void*>(&choice));
    if (!prepared_field || !prepared_choice) { log("Could not allocate branch thunks"); return false; }
#ifdef OT0_DEVELOPMENT
    prepared_update = thunk(build::UpdateCall, reinterpret_cast<void*>(&update_probe));
    if (!prepared_update) return false;
#endif
    return true;
}
bool activate_hooks() {
    if (!patch_call(build::ChoiceCall, prepared_choice) || !patch_call(build::FieldCall, prepared_field)
#ifdef OT0_DEVELOPMENT
        || !patch_call(build::UpdateCall, prepared_update)
#endif
        ) {
        patch_call(build::ChoiceCall, image + build::Choice);
        patch_call(build::FieldCall, image + build::Standby);
#ifdef OT0_DEVELOPMENT
        patch_call(build::UpdateCall, image + build::Update);
#endif
        failed = true; ordinary_scope = false; return false;
    }
    installed = true;
    return true;
}
bool initialize() {
    if (installed) return true;
    bool ok = prepare_hooks() && activate_hooks();
    log(ok ? "Native module installed" : "Hook installation failed", margin, enabled);
    return ok;
}
}

extern "C" __declspec(dllexport) int ot0_initialize(void*) { initialize(); return 0; }
#ifdef OT0_DEVELOPMENT
extern "C" __declspec(dllexport) u32 ot0_development_initialize(void* parameter) {
    auto* gate = static_cast<volatile u32*>(parameter);
    bool ready = prepare_hooks();
    gate[0] = 1;
    while (!gate[1]) Sleep(1);
    if (gate[1] == 2) return 2;
    return ready && activate_hooks() ? 0 : 1;
}
extern "C" __declspec(dllexport) u32 ot0_development_shutdown(void* parameter) {
    auto* gate = static_cast<volatile u32*>(parameter);
    gate[0] = 1;
    while (!gate[1]) Sleep(1);
    if (gate[1] == 2) return 2;
    enabled = false;
    bool ok = patch_call(build::FieldCall, image + build::Standby)
        && patch_call(build::ChoiceCall, image + build::Choice);
    ok = patch_call(build::UpdateCall, image + build::Update) && ok;
    installed = false;
    return ok ? 0 : 1;
}
#endif
extern "C" __declspec(dllexport) int ot0_toggle(void*) {
    if (!installed || failed) return 0;
    settings(); enabled = !enabled;
    update_language();
    WritePrivateProfileStringW(L"General", L"Enabled", enabled ? L"1" : L"0", config_path);
    notice(chinese ? (traditional ? L"自動結算" : L"自动结算") : L"Auto Resolve",
        chinese ? (traditional ? (enabled ? L"已開啟" : L"已關閉") : (enabled ? L"已开启" : L"已关闭"))
            : (enabled ? L"Enabled" : L"Disabled"));
    return 0;
}
extern "C" int DllMain(Handle module, u32 reason, void*) { if (reason == 1) library = module; return 1; }
