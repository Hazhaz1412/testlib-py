#include <pybind11/pybind11.h>
#include "../testlib/testlib.h"

namespace py = pybind11;

namespace {
    bool registered = false;

    void ensureRegistered() {
        if (!registered) {
            static char name[] = "sgen";
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
    return rnd.next("[a-zA-Z0-9]{1,1000}");
}

PYBIND11_MODULE(sgen, m) {
    m.doc() = "Random alphanumeric token of length 1..1000.";
    m.def("generate", &generate, py::arg("seed") = py::none(),
          "Return a random token matching [a-zA-Z0-9]{1,1000}.");
}
