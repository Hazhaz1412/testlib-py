#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../testlib/testlib.h"

#include <sstream>
#include <utility>
#include <vector>

namespace py = pybind11;

namespace {
    bool registered = false;

    void ensureRegistered() {
        if (!registered) {
            static char name[] = "gen_tree_graph";
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

    std::vector<int> perm = rnd.perm(n);

    std::vector<std::pair<int, int>> edges;
    edges.reserve(n - 1);
    for (int i = 1; i < n; ++i) {
        if (rnd.next(2)) {
            edges.emplace_back(perm[i], perm[p[i]]);
        } else {
            edges.emplace_back(perm[p[i]], perm[i]);
        }
    }

    shuffle(edges.begin(), edges.end());

    std::ostringstream out;
    out << n << "\n";
    for (const auto &edge : edges) {
        out << edge.first + 1 << ' ' << edge.second + 1 << "\n";
    }

    return out.str();
}

PYBIND11_MODULE(gen_tree_graph, m) {
    m.doc() = "Random tree generator producing an edge list.";
    m.def("generate",
          &generate,
          py::arg("n"),
          py::arg("t"),
          py::arg("seed") = py::none(),
          "Return the testlib text output for a random tree of size n.");
}
