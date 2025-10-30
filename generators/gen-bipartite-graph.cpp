#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../testlib/testlib.h"

#include <set>
#include <sstream>
#include <utility>
#include <vector>

namespace py = pybind11;

namespace {
    bool registered = false;

    void ensureRegistered() {
        if (!registered) {
            static char name[] = "gen_bipartite_graph";
            static char *argv[] = {name};
            registerGen(1, argv, 1);
            registered = true;
        }
    }
}

std::string generate(int n, int m, std::size_t k, py::object seed = py::none()) {
    ensureRegistered();

    if (n <= 0) {
        throw py::value_error("n must be positive");
    }
    if (m <= 0) {
        throw py::value_error("m must be positive");
    }

    const std::size_t max_edges = static_cast<std::size_t>(n) * static_cast<std::size_t>(m);
    if (k > max_edges) {
        throw py::value_error("k must not exceed n * m");
    }

    if (!seed.is_none()) {
        rnd.setSeed(seed.cast<long long>());
    }

    int t = rnd.next(-2, 2);

    std::set<std::pair<int, int>> edges;
    while (edges.size() < k) {
        int a = rnd.wnext(n, t);
        int b = rnd.wnext(m, t);
        edges.insert(std::make_pair(a, b));
    }

    std::vector<std::pair<int, int>> e(edges.begin(), edges.end());
    shuffle(e.begin(), e.end());

    std::vector<int> pa = rnd.perm(n, 1);
    std::vector<int> pb = rnd.perm(m, 1);

    std::ostringstream out;
    out << n << ' ' << m << ' ' << e.size() << "\n";
    for (const auto &edge : e) {
        out << pa[edge.first] << ' ' << pb[edge.second] << "\n";
    }

    return out.str();
}

PYBIND11_MODULE(gen_bipartite_graph, m) {
    m.doc() = "Random bipartite graph generator.";
    m.def("generate",
          &generate,
          py::arg("n"),
          py::arg("m"),
          py::arg("k"),
          py::arg("seed") = py::none(),
          "Generate a bipartite graph in the original testlib text format.\n"
          "n, m specify partition sizes, k is the number of edges.");
}
