#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../testlib/testlib.h"

#include <sstream>
#include <vector>

namespace py = pybind11;

namespace {
    bool registered = false;

    void ensureRegistered() {
        if (!registered) {
            static char name[] = "gen_rooted_tree_graph";
            static char *argv[] = {name};
            registerGen(1, argv, 1);
            registered = true;
        }
    }
}

std::string generate(int n, int t, py::object seed = py::none()) {
    ensureRegistered();

    if (n <= 0) {
        throw py::value_error("n must be positive");
    }

    if (!seed.is_none()) {
        rnd.setSeed(seed.cast<long long>());
    }

    std::vector<int> p(n);
    for (int i = 1; i < n; ++i) {
        p[i] = rnd.wnext(i, t);
    }

    std::vector<int> perm(n);
    for (int i = 0; i < n; ++i) {
        perm[i] = i;
    }
    shuffle(perm.begin() + 1, perm.end());

    std::vector<int> pp(n - 1);
    for (int i = 1; i < n; ++i) {
        pp[perm[i] - 1] = perm[p[i]] + 1;
    }

    std::ostringstream out;
    out << n << "\n";
    for (std::size_t i = 0; i < pp.size(); ++i) {
        out << pp[i];
        if (i + 1 < pp.size()) {
            out << ' ';
        }
    }
    out << "\n";

    return out.str();
}

PYBIND11_MODULE(gen_rooted_tree_graph, m) {
    m.doc() = "Random rooted tree generator producing parent list.";
    m.def("generate",
          &generate,
          py::arg("n"),
          py::arg("t"),
          py::arg("seed") = py::none(),
          "Return the testlib text output for a rooted tree of size n.");
}
