"""Build the freestanding Windows x64 module with Clang and LLVM lld."""

import pathlib
import shutil
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
OUT = ROOT / ".local" / "build"
OUT.mkdir(parents=True, exist_ok=True)


def find_tool(name, homebrew_path):
    found = shutil.which(name)
    if found:
        return found
    if pathlib.Path(homebrew_path).is_file():
        return homebrew_path
    raise SystemExit(f"Required build tool not found: {name}")


clang = find_tool("clang++", "/opt/homebrew/opt/llvm/bin/clang++")
linker = find_tool("lld-link", "/opt/homebrew/opt/lld/bin/lld-link")
dlltool = find_tool("llvm-dlltool", "/opt/homebrew/opt/llvm/bin/llvm-dlltool")
imports = [
    "GetModuleHandleW", "GetModuleHandleExW", "GetModuleFileNameW", "VirtualAlloc", "VirtualProtect",
    "FlushInstructionCache", "GetCurrentProcess", "GetPrivateProfileIntW",
    "WritePrivateProfileStringW", "CreateFileW", "WriteFile", "CloseHandle",
]
development = "--development" in sys.argv[1:]
if development:
    imports.append("Sleep")
definition = OUT / "kernel32.def"
definition.write_text("LIBRARY KERNEL32.dll\nEXPORTS\n" + "\n".join(imports) + "\n")
subprocess.run([dlltool, "-m", "i386:x86-64", "-d", str(definition), "-l", str(OUT / "kernel32.lib")], check=True)
obj = OUT / "main.obj"
subprocess.run([
    clang, "--target=x86_64-pc-windows-msvc", "-std=c++17", "-O2",
    "-ffreestanding", "-fno-exceptions", "-fno-rtti", "-fno-stack-protector",
    "-fno-builtin", "-mno-stack-arg-probe", "-Wall", "-Wextra", "-Werror",
    *(["-DOT0_DEVELOPMENT=1"] if development else []),
    "-c", str(ROOT / "native/main.cpp"), "-o", str(obj),
], check=True)
subprocess.run([
    linker, "/dll", "/entry:DllMain", "/nodefaultlib", "/machine:x64",
    "/dynamicbase", "/nxcompat", "/timestamp:0",
    f"/out:{OUT / 'ot0_native.dll'}", str(obj), str(OUT / "kernel32.lib"),
], check=True)
print(OUT / "ot0_native.dll")
