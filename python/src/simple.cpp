#include <pybind11/pybind11.h>
#include "brica/brica.hpp"

namespace py = pybind11;

using Dict = brica::AssocVec<std::string, py::object>;
using Functor = std::function<void(Dict&, Dict&)>;

using ComponentBase = brica::ComponentBase<py::object, Dict, Functor>;

Functor wrap_function(py::object f) {
  return [f](Dict& inputs, Dict& outputs) {
    py::dict args;
    for (auto it = inputs.begin(); it != inputs.end(); ++it) {
      args[py::cast(it->first)] = it->second;
    }
    py::dict rets = f(args);
    for (auto it = outputs.begin(); it != outputs.end(); ++it) {
      it->second = rets[py::cast(it->first)];
    }
  };
}

class Component : public ComponentBase {
  static py::none None;

 public:
  Component(py::object f) : ComponentBase(wrap_function(f)) {}
  ~Component() {}

  void make_in_port(std::string name) {
    ComponentBase::make_in_port(name);
    in_port.at(name)->set(None);
    inputs.at(name) = None;
  }

  void make_out_port(std::string name) {
    ComponentBase::make_out_port(name);
    out_port.at(name)->set(None);
    outputs.at(name) = None;
  }

  py::object get_in_port_value(std::string name) {
    return in_port.at(name)->get();
  }

  py::object get_out_port_value(std::string name) {
    return out_port.at(name)->get();
  }

  py::object get_input(std::string name) { return inputs.at(name); }
  py::object get_output(std::string name) { return outputs.at(name); }
};

py::none Component::None;

PYBIND11_MODULE(brica, m) {
  py::class_<brica::IComponent>(m, "IComponent");
  py::class_<ComponentBase, brica::IComponent>(m, "ComponentBase");
  py::class_<Component, ComponentBase>(m, "Component")
      .def(py::init<py::object>())
      .def("make_in_port", &Component::make_in_port)
      .def("make_out_port", &Component::make_out_port)
      .def("get_in_port_value", &Component::get_in_port_value)
      .def("get_out_port_value", &Component::get_out_port_value)
      .def("get_input", &Component::get_input)
      .def("get_output", &Component::get_output)
      .def("collect", &Component::collect)
      .def("execute", &Component::execute)
      .def("expose", &Component::expose);

  m.def("connect", &brica::connect<Component>);

  py::class_<brica::Timing>(m, "Timing")
      .def(py::init<brica::Time, brica::Time, brica::Time>());

  py::class_<brica::VirtualTimeScheduler>(m, "VirtualTimeScheduler")
      .def(py::init<>())
      .def("add_component", &brica::VirtualTimeScheduler::add_component)
      .def("step", &brica::VirtualTimeScheduler::step);
}
