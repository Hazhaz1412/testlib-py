# testlib-py

A Python-friendly port of the original [testlib](https://github.com/MikeMirzayanov/testlib) toolkit.
Besides bundling the C++ headers, this fork exposes several classic generators as
Python extension modules via **pybind11**.

## Prerequisites

- Python 3.11 (the repo ships a `py311` conda environment in examples below)
- `pybind11` and `setuptools` installed in that environment
- A C++17-capable compiler reachable from the shell (`g++/clang` on Linux/macOS, MSVC on Windows)

## Building the generator modules

1. Activate your environment (adjust the command if you use another env manager):
   ```bash
   conda activate py311
   ```
2. Run the terminal UI that compiles every pybind11 generator:
   ```bash
   cd testlib-py/tools
   python generator_builder.py
   # or use the English UI
   python generator_builder_en.py
   ```
3. Inside the menu choose option `1` (“Build tat ca module”) to compile everything.

Upon success the script copies every compiled module into `testlib-py/builds/`.
Whenever you edit a generator, rerun either `python generator_builder.py` or `python generator_builder_en.py` to refresh that folder.

## Using the generators from Python

Make sure Python can locate the `builds/` directory (for example run scripts from the
project root or `export PYTHONPATH=$PWD/builds:$PYTHONPATH`). Then import and call the modules just like
regular Python packages:

```python
import bgen
import gen_array_with_opt

print(bgen.generate())                # random 100-bit string
print(gen_array_with_opt.generate(2, 5, seed=123))
```

### Available modules and signatures

| Module | Function signature | Description |
| ------ | ------------------ | ----------- |
| `bgen` | `generate(seed: int | None = None) -> str` | 100-character binary string with roughly 10% ones. |
| `gen_array_with_opt` | `generate(test_count, sum_n, min_n=1, min_value=1, max_value=1_000_000_000, value_bias=0, seed=None) -> str` | Recreates the CLI output of `gen-array-with-opt`, returning the formatted multi-test string. |
| `gen_bipartite_graph` | `generate(n, m, k, seed=None) -> str` | Random bipartite graph with partitions of size `n` and `m` and exactly `k` edges. |
| `gen_rooted_tree_graph` | `generate(n, t, seed=None) -> str` | Rooted tree generator; output is `n` followed by the parent list layout used by testlib. |
| `gen_tree_graph` | `generate(n, t, seed=None) -> str` | Random tree generator that returns `n` and an edge list. |
| `gs` | `generate(parts: list[tuple[int, str]]) -> str` | Deterministic concatenation utility; each pair is `(repeat_count, pattern)`. |
| `igen` | `generate(seed=None) -> int` | Uniform random integer in `[1, 1_000_000]`. |
| `iwgen` | `generate(weight=0, seed=None) -> int` | Weighted random integer using `rnd.wnext` over `[1, 1_000_000]`. |
| `sgen` | `generate(seed=None) -> str` | Random alphanumeric string matching `[a-zA-Z0-9]{1,1000}`. |
| `swgen` | `generate(weight=0, seed=None) -> str` | Weighted string generator; `weight` biases the length distribution (negative for shorter, positive for longer). |

Each function mirrors the original testlib semantics—passing a `seed` value (when
available) makes the output deterministic. Without a seed, the underlying
`testlib::rnd` continues from its current state.

## Notes

- On Windows the build artifacts are `.pyd` files. Install the MSVC Build Tools (or
  another C++17 compiler) before running `python generator_builder.py` so pybind11 can
  produce the correct binaries.
- Additional testlib components (checkers, interactors, validators) remain in C++ form and
  can be consumed directly from that language.
- If you add your own generators under `generators/` or `testlib/generators/` and wrap them
  with `PYBIND11_MODULE`, the builder script will discover and compile them automatically.
