#pragma once

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;
using i64 = long long;
using Handle = void*;
using WChar = wchar_t;

#define WIN_IMPORT extern "C" __declspec(dllimport)
WIN_IMPORT Handle GetModuleHandleW(const WChar*);
WIN_IMPORT int GetModuleHandleExW(u32, const WChar*, Handle*);
WIN_IMPORT u32 GetModuleFileNameW(Handle, WChar*, u32);
WIN_IMPORT void* VirtualAlloc(void*, u64, u32, u32);
WIN_IMPORT int VirtualProtect(void*, u64, u32, u32*);
WIN_IMPORT int FlushInstructionCache(Handle, const void*, u64);
WIN_IMPORT Handle GetCurrentProcess();
WIN_IMPORT u32 GetPrivateProfileIntW(const WChar*, const WChar*, int, const WChar*);
WIN_IMPORT int WritePrivateProfileStringW(const WChar*, const WChar*, const WChar*, const WChar*);
WIN_IMPORT Handle CreateFileW(const WChar*, u32, u32, void*, u32, u32, Handle);
WIN_IMPORT int WriteFile(Handle, const void*, u32, u32*, void*);
WIN_IMPORT int CloseHandle(Handle);
#ifdef OT0_DEVELOPMENT
WIN_IMPORT void Sleep(u32);
#endif

template<class T> T& field(void* object, u32 offset) {
    return *reinterpret_cast<T*>(static_cast<u8*>(object) + offset);
}
template<class T> const T& field(const void* object, u32 offset) {
    return *reinterpret_cast<const T*>(static_cast<const u8*>(object) + offset);
}
template<class T> struct Array {
    T* data;
    int count;
    int capacity;
    bool valid(int limit) const {
        return count >= 0 && count <= limit && capacity >= count && (!count || data);
    }
};
static_assert(sizeof(Array<int>) == 16);
