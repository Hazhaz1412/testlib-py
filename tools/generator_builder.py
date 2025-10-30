#!/usr/bin/env python3
"""
Interactive terminal UI for building pybind11 generator modules.

The script scans generator directories for C++ sources that expose a
`PYBIND11_MODULE` definition and compiles them in place. It relies on
setuptools/pybind11, so builds work across Linux, macOS, and Windows as
long as a suitable C++17 compiler is available in the active Python
environment.
"""

from __future__ import annotations

import os
import platform
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import List, Sequence

try:
    from setuptools import Distribution
except ImportError as exc:  # pragma: no cover - informative message
    raise SystemExit("setuptools is required to run this builder.") from exc

try:
    from pybind11.setup_helpers import Pybind11Extension, build_ext
except ImportError as exc:  # pragma: no cover - informative message
    raise SystemExit(
        "pybind11 is required. Install it with `pip install pybind11` "
        "inside your active Python environment."
    ) from exc


ROOT_DIR = Path(__file__).resolve().parents[1]
GENERATOR_ROOTS = [ROOT_DIR / "generators", ROOT_DIR / "testlib" / "generators"]
LOCAL_PYBIND11_INCLUDE = ROOT_DIR / "pybind11" / "include"
BUILD_OUTPUT_DIR = ROOT_DIR / "builds"


@dataclass(frozen=True)
class ModuleSpec:
    name: str
    source: Path

    @property
    def extension(self) -> Pybind11Extension:
        include_dirs = {str(self.source.parent)}
        testlib_dir = self.source.parent.parent / "testlib"
        if testlib_dir.exists():
            include_dirs.add(str(testlib_dir))
        if LOCAL_PYBIND11_INCLUDE.exists():
            include_dirs.add(str(LOCAL_PYBIND11_INCLUDE))
        return Pybind11Extension(
            self.name,
            [str(self.source)],
            include_dirs=sorted(include_dirs),
            cxx_std=17,
        )


def discover_modules() -> List[ModuleSpec]:
    modules: List[ModuleSpec] = []
    pattern = re.compile(r"PYBIND11_MODULE\(\s*([A-Za-z0-9_]+)")
    for root in GENERATOR_ROOTS:
        if not root.exists():
            continue
        for source in sorted(root.glob("*.cpp")):
            try:
                text = source.read_text(encoding="utf-8")
            except UnicodeDecodeError:
                continue
            match = pattern.search(text)
            if not match:
                continue
            module_name = match.group(1)
            modules.append(ModuleSpec(name=module_name, source=source))
    modules.sort(key=lambda spec: spec.name)
    return modules


def build_modules(modules: Sequence[ModuleSpec]) -> None:
    if not modules:
        print("Khong co module nao de build.")
        return

    dist = Distribution()
    dist.script_name = "generator_builder"
    dist.ext_modules = [spec.extension for spec in modules]
    dist.cmdclass = {"build_ext": build_ext}

    cmd = build_ext(dist)
    cmd.force = True
    cmd.inplace = True
    cmd.initialize_options()
    cmd.finalize_options()

    try:
        cmd.run()
    except Exception as exc:  # pragma: no cover - surfaced to user
        print(f"\nBuild that bai: {exc}\n")
        raise

    BUILD_OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    for spec in modules:
        built_path = Path(cmd.get_ext_fullpath(spec.name))
        if not built_path.exists():
            continue
        target = BUILD_OUTPUT_DIR / built_path.name
        target.write_bytes(built_path.read_bytes())


def print_banner(modules: Sequence[ModuleSpec]) -> None:
    os_name = platform.system()
    print("=" * 60)
    print(" Testlib Generator Builder (pybind11)")
    print("=" * 60)
    print(f"He dieu hanh: {os_name}")
    roots = [root for root in GENERATOR_ROOTS if root.exists()]
    for root in roots:
        print(f"Thu muc: {root}")
    print(f"Tim thay {len(modules)} module(s) pybind11.\n")


def prompt_selection(modules: Sequence[ModuleSpec]) -> List[ModuleSpec]:
    if not modules:
        print("Chua co generator nao dung pybind11. Hay chuyen doi truoc.")
        return []

    for idx, spec in enumerate(modules, start=1):
        print(f"{idx:>2}. {spec.name:<25} ({spec.source.name})")
    print(" a. Build tat ca")
    print(" q. Quay lai\n")

    choice = input("Chon module (vi du: 1 3 5, hoac 'a'): ").strip().lower()
    if choice in {"q", ""}:
        return []
    if choice in {"a", "all"}:
        return list(modules)

    selected: List[ModuleSpec] = []
    tokens = re.split(r"[,\s]+", choice)
    for token in tokens:
        if not token:
            continue
        if not token.isdigit():
            print(f"Bo qua lua chon khong hop le: {token}")
            continue
        idx = int(token)
        if 1 <= idx <= len(modules):
            selected.append(modules[idx - 1])
        else:
            print(f"Bo qua chi muc ngoai pham vi: {token}")
    return selected


def interactive_loop() -> None:
    while True:
        modules = discover_modules()
        print_banner(modules)
        print("Tuy chon:")
        print(" 1. Build tat ca module")
        print(" 2. Chon module de build")
        print(" 3. Liet ke module hien co")
        print(" 0. Thoat\n")

        choice = input("Nhap lua chon: ").strip()
        if choice == "0":
            print("Tam biet!")
            return
        if choice == "1":
            selected = modules
        elif choice == "2":
            selected = prompt_selection(modules)
        elif choice == "3":
            _ = prompt_selection(modules)
            input("\nNhan Enter de quay lai menu chinh...")
            continue
        else:
            print("Lua chon khong hop le.\n")
            continue

        if not selected:
            print("Khong co module nao duoc chon.\n")
            continue

        print("\nBat dau build:")
        for spec in selected:
            print(f" - {spec.name} ({spec.source})")
        print()

        try:
            build_modules(selected)
        except Exception:
            input("Nhan Enter de quay lai menu...")
            continue

        print(f"\nBuild hoan tat. File .so/.pyd nam trong thu muc: {BUILD_OUTPUT_DIR}\n")
        input("Nhan Enter de quay lai menu...")


def main(argv: Sequence[str]) -> None:
    if len(argv) > 1 and argv[1] in {"-h", "--help"}:
        print(__doc__)
        return
    interactive_loop()


if __name__ == "__main__":
    main(sys.argv)
