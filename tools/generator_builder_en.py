#!/usr/bin/env python3
"""English terminal UI for building pybind11 generator modules."""

from __future__ import annotations

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
        print("No modules selected for build.")
        return

    dist = Distribution()
    dist.script_name = "generator_builder_en"
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
        print(f"
Build failed: {exc}
")
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
    print(" Testlib Generator Builder (pybind11) – English UI")
    print("=" * 60)
    print(f"Operating system: {os_name}")
    roots = [root for root in GENERATOR_ROOTS if root.exists()]
    for root in roots:
        print(f"Scanning: {root}")
    print(f"Detected {len(modules)} pybind11 module(s).
")


def prompt_selection(modules: Sequence[ModuleSpec]) -> List[ModuleSpec]:
    if not modules:
        print("No pybind11 generators detected. Convert your C++ files first.")
        return []

    for idx, spec in enumerate(modules, start=1):
        print(f"{idx:>2}. {spec.name:<25} ({spec.source.name})")
    print(" a. Build all")
    print(" q. Back
")

    choice = input("Select modules (e.g. 1 3 5, or 'a'): ").strip().lower()
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
            print(f"Ignoring invalid entry: {token}")
            continue
        idx = int(token)
        if 1 <= idx <= len(modules):
            selected.append(modules[idx - 1])
        else:
            print(f"Ignoring out-of-range index: {token}")
    return selected


def interactive_loop() -> None:
    while True:
        modules = discover_modules()
        print_banner(modules)
        print("Options:")
        print(" 1. Build all modules")
        print(" 2. Pick modules to build")
        print(" 3. List modules")
        print(" 0. Exit
")

        choice = input("Enter your choice: ").strip()
        if choice == "0":
            print("Goodbye!")
            return
        if choice == "1":
            selected = modules
        elif choice == "2":
            selected = prompt_selection(modules)
        elif choice == "3":
            _ = prompt_selection(modules)
            input("
Press Enter to return to the main menu...")
            continue
        else:
            print("Invalid choice.
")
            continue

        if not selected:
            print("No modules selected.
")
            continue

        print("
Starting build:")
        for spec in selected:
            print(f" - {spec.name} ({spec.source})")
        print()

        try:
            build_modules(selected)
        except Exception:
            input("Press Enter to return to the menu...")
            continue

        print(f"
Build complete. Shared libraries copied to: {BUILD_OUTPUT_DIR}
")
        input("Press Enter to return to the menu...")


def main(argv: Sequence[str]) -> None:
    if len(argv) > 1 and argv[1] in {"-h", "--help"}:
        print(__doc__)
        return
    interactive_loop()


if __name__ == "__main__":
    main(sys.argv)
