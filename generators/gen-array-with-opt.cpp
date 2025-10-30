/**
 * gen-array-with-opt   -test-count <num>
 *                      -sum-n <num>
 *                      [-min-n <num>]
 *                      [-min-value <num>] [-max-value <num>]
 *                      [-value-bias <num>]
 *
 * Converted into a pybind11 module so the generator can be used from Python.
 */

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
            static char name[] = "gen_array_with_opt";
            static char *argv[] = {name};
            registerGen(1, argv, 1);
            registered = true;
        }
    }
}

std::string generate(int test_count,
                     int sum_n,
                     int min_n = 1,
                     int min_value = 1,
                     int max_value = 1000000000,
                     int value_bias = 0,
                     py::object seed = py::none()) {
    ensureRegistered();

    if (test_count <= 0) {
        throw py::value_error("test_count must be positive");
    }
    if (min_n <= 0) {
        throw py::value_error("min_n must be positive");
    }
    if (static_cast<long long>(test_count) * min_n > sum_n) {
        throw py::value_error("sum_n must be at least test_count * min_n");
    }
    if (min_value > max_value) {
        throw py::value_error("min_value must be <= max_value");
    }

    if (!seed.is_none()) {
        rnd.setSeed(seed.cast<long long>());
    }

    std::vector<int> n_list = rnd.partition(test_count, sum_n, min_n);

    std::ostringstream out;
    out << test_count << "\n";
    for (int test_case = 0; test_case < test_count; ++test_case) {
        int n = n_list[test_case];
        out << n << "\n";
        for (int i = 0; i < n; ++i) {
            int value = rnd.wnext(min_value, max_value, value_bias);
            out << value;
            if (i + 1 < n) {
                out << ' ';
            }
        }
        out << "\n";
    }

    return out.str();
}

PYBIND11_MODULE(gen_array_with_opt, m) {
    m.doc() = "Array generator with configurable parameters.";
    m.def("generate",
          &generate,
          py::arg("test_count"),
          py::arg("sum_n"),
          py::arg("min_n") = 1,
          py::arg("min_value") = 1,
          py::arg("max_value") = 1000000000,
          py::arg("value_bias") = 0,
          py::arg("seed") = py::none(),
          "Generate arrays formatted like the original CLI tool.\n"
          "Optional seed allows deterministic output.");
}
