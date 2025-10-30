#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <sstream>
#include <string>
#include <vector>

namespace py = pybind11;

std::string generate(const std::vector<std::pair<int, std::string>> &parts) {
    if (parts.empty()) {
        throw py::value_error("parts must not be empty");
    }

    std::ostringstream out;
    for (const auto &part : parts) {
        int repeat = part.first;
        const std::string &pattern = part.second;
        if (repeat < 0) {
            throw py::value_error("repeat count must be non-negative");
        }
        for (int i = 0; i < repeat; ++i) {
            out << pattern;
        }
    }

    return out.str();
}

PYBIND11_MODULE(gs, m) {
    m.doc() = "Concatenate repeating string parts (deterministic generator).";
    m.def("generate", &generate, py::arg("parts"),
          "Build a string from a sequence of (repeat_count, pattern) pairs.");
}
