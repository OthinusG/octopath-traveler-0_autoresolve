"""Build mod-only and source archives from explicit project-owned files."""
import hashlib
import pathlib
import subprocess
import zipfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
VERSION = "0.1.0"
subprocess.run(["python3", str(ROOT / "tools/build_native.py")], check=True)
binary = ROOT / ".local/build/ot0_native.dll"
# Development helpers have additional exports and must not enter distribution.
payload = binary.read_bytes()
assert b"ot0_development_" not in payload
assert b"ShortInterval" not in payload and b"Preview" not in payload
out = ROOT / "dist"
out.mkdir(exist_ok=True)
archives = []
for source in (False, True):
    path = out / (f"OT0AutoResolve-{VERSION}-source.zip" if source else f"OT0AutoResolve-{VERSION}.zip")
    files = {name: ROOT / name for name in ("README.md", "VALIDATION.md")}
    for relative in ("enabled.txt", "config.ini", "Scripts/main.lua"):
        file = ROOT / "mod/OT0AutoResolve" / relative
        files[str(file.relative_to(ROOT if source else ROOT / "mod"))] = file
    if source:
        for directory in ("native", "tests", "tools"):
            for file in sorted((ROOT / directory).glob("*")):
                if file.is_file() and file.suffix in (".hpp", ".cpp", ".lua", ".py"):
                    files[str(file.relative_to(ROOT))] = file
        for name in ("AGENTS.md", "MEMORY.md", "README.en.md", "NOTICE.md", "mod/README.md",
                     "docs/Tech-Spec.md", "docs/DEVELOPMENT.md", "docs/VALIDATION.md",
                     "LICENSES/UE4SS-MIT.txt", "images/icon.png", "images/ot0rs_en.png", "images/ot0rs_zh.png"):
            files[name] = ROOT / name
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as archive:
        for name, file in files.items():
            archive.write(file, name)
        if not source:
            archive.writestr("OT0AutoResolve/Scripts/ot0_native.dll", payload)
    with zipfile.ZipFile(path) as archive:
        assert archive.testzip() is None
        assert all(not name.startswith("/") and ".." not in pathlib.PurePosixPath(name).parts for name in archive.namelist())
        if not source:
            assert archive.read("OT0AutoResolve/Scripts/ot0_native.dll") == payload
    archives.append(path)
(out / "SHA256SUMS.txt").write_text("".join(
    hashlib.sha256(path.read_bytes()).hexdigest() + "  " + path.name + "\n" for path in archives
))
for path in archives:
    print(path)
