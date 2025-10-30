/*
 * Outputs random 100-digit binary string mostly containing 0's.
 * In average, it contains only 10% of 1's.
 *
 * Converted into a pybind11 module to expose the generator to Python.
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../testlib/testlib.h"

namespace py = pybind11;

namespace {
    bool registered = false;

    void ensureRegistered() {
        if (!registered) {
            static char name[] = "bgen";
            static char *argv[] = {name};
            registerGen(1, argv, 1);
            registered = true;
        }
    }
}

std::string generate(py::object seed = py::none()) {
    ensureRegistered();
    if (!seed.is_none()) {
        rnd.setSeed(seed.cast<long long>());
    }
    return rnd.next("[0000000001]{100}");
}

PYBIND11_MODULE(bgen, m) {
    m.doc() = "Binary string generator (~10% ones).";
    m.def("generate",
          &generate,
          py::arg("seed") = py::none(),
          "Return a random length-100 binary string with roughly 10% ones.\n"
          "Optionally provide an integer seed for deterministic output.");
}
