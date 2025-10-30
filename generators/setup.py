
from pathlib import Path
import re

from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup

ROOT = Path(__file__).resolve().parent
TESTLIB_INCLUDE = ROOT.parent / "testlib"

pattern = re.compile(r"PYBIND11_MODULE\(\s*([A-Za-z0-9_]+)")

def discover_extensions():
    exts = []
    for source in sorted(ROOT.glob("*.cpp")):
        text = source.read_text(encoding="utf-8")
        match = pattern.search(text)
        if not match:
            continue
        name = match.group(1)
        exts.append(
            Pybind11Extension(
                name,
                [str(source)],
                include_dirs=[str(TESTLIB_INCLUDE)],
                cxx_std=17,
            )
        )
    return exts

setup(
    name="testlib-generators",
    version="0.1.0",
    description="Pybind11 bindings for testlib generators",
    ext_modules=discover_extensions(),
    cmdclass={"build_ext": build_ext},
)
