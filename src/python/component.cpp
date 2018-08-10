#include <pybind11/pybind11.h>
#include "brica/brica.hpp"

namespace py = pybind11;

brica::Functor wrap_function(py::object f) {
  return [f](brica::Dict& inputs, brica::Dict& outputs) {
    py::dict pyinputs;
    for (auto it = inputs.begin(); it != inputs.end(); ++it) {
      pyinputs[py::cast(it->first)] = py::cast(it->second);
    }
    py::dict pyoutputs = f(pyinputs);
  };
}

class Component : public brica::Component {
 public:
  Component(py::object f) : brica::Component(wrap_function(f)) {}
};

PYBIND11_MODULE(brica, m) {
  py::class_<brica::IComponent>(m, "IComponent");

  py::class_<Component, brica::IComponent>(m, "Component")
      .def(py::init<py::object>())
      .def("make_in_port", &Component::make_in_port)
      .def("make_out_port", &Component::make_out_port);

  m.def("connect", &brica::connect<Component>);

  py::class_<brica::Timing>(m, "Timing")
      .def(py::init<brica::Time, brica::Time, brica::Time>());

  py::class_<brica::VirtualTimeScheduler>(m, "VirtualTimeScheduler")
      .def(py::init<>())
      .def("add_component", &brica::VirtualTimeScheduler::add_component)
      .def("step", &brica::VirtualTimeScheduler::step);
}
