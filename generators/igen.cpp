#include <pybind11/pybind11.h>
#include "../testlib/testlib.h"

namespace py = pybind11;

namespace {
    bool registered = false;

    void ensureRegistered() {
        if (!registered) {
            static char name[] = "igen";
            static char *argv[] = {name};
            registerGen(1, argv, 1);
            registered = true;
        }
    }
}

int generate(py::object seed = py::none()) {
    ensureRegistered();
    if (!seed.is_none()) {
        rnd.setSeed(seed.cast<long long>());
    }
    return rnd.next(1, 1000000);
}

PYBIND11_MODULE(igen, m) {
    m.doc() = "Uniform random integer in [1, 10^6].";
    m.def("generate", &generate, py::arg("seed") = py::none(),
          "Return a random integer between 1 and 1_000_000.");
}
