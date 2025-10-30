#include <pybind11/pybind11.h>
#include "../testlib/testlib.h"

namespace py = pybind11;

namespace {
    bool registered = false;

    void ensureRegistered() {
        if (!registered) {
            static char name[] = "swgen";
            static char *argv[] = {name};
            registerGen(1, argv, 1);
            registered = true;
        }
    }
}

std::string generate(int weight = 0, py::object seed = py::none()) {
    ensureRegistered();
    if (!seed.is_none()) {
        rnd.setSeed(seed.cast<long long>());
    }
    int length = rnd.wnext(1, 1000, weight);
    return rnd.next("[a-zA-Z0-9]{1,%d}", length);
}

PYBIND11_MODULE(swgen, m) {
    m.doc() = "Weighted random alphanumeric token generator.";
    m.def("generate", &generate,
          py::arg("weight") = 0,
          py::arg("seed") = py::none(),
          "Return a random token with length distribution controlled by weight.");
}
