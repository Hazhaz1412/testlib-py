#include <pybind11/pybind11.h>
#include "../testlib/testlib.h"

namespace py = pybind11;

namespace {
    bool registered = false;

    void ensureRegistered() {
        if (!registered) {
            static char name[] = "iwgen";
            static char *argv[] = {name};
            registerGen(1, argv, 1);
            registered = true;
        }
    }
}

int generate(int weight = 0, py::object seed = py::none()) {
    ensureRegistered();
    if (!seed.is_none()) {
        rnd.setSeed(seed.cast<long long>());
    }
    return rnd.wnext(1, 1000000, weight);
}

PYBIND11_MODULE(iwgen, m) {
    m.doc() = "Weighted random integer in [1, 10^6].";
    m.def("generate", &generate,
          py::arg("weight") = 0,
          py::arg("seed") = py::none(),
          "Return a weighted random integer (uses testlib::rnd.wnext).");
}
