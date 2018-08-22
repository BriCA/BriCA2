#include <pybind11/pybind11.h>
#include "brica/brica.hpp"

namespace py = pybind11;

using Dict = brica::AssocVec<std::string, py::object>;
using Functor = std::function<void(Dict&, Dict&)>;

using ComponentBase = brica::ComponentBase<py::object>;

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

class Component final : public ComponentBase {
 public:
  Component(py::object f) : ComponentBase(wrap_function(f)) {}
  ~Component() {}

  void make_in_port(std::string name) {
    ComponentBase::make_in_port(name);
    py::none none;
    in_port.at(name)->set(none);
    inputs.at(name) = none;
  }

  void make_out_port(std::string name) {
    ComponentBase::make_out_port(name);
    py::none none;
    out_port.at(name)->set(none);
    outputs.at(name) = none;
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

PYBIND11_MODULE(brica, m) {
  py::class_<brica::IComponent>(m, "IComponent")
      .def("collect", &brica::IComponent::collect)
      .def("execute", &brica::IComponent::execute)
      .def("expose", &brica::IComponent::expose);

  py::class_<ComponentBase, brica::IComponent>(m, "ComponentBase")
      .def("make_in_port", &ComponentBase::make_in_port)
      .def("make_out_port", &ComponentBase::make_out_port);

  py::class_<Component, ComponentBase>(m, "Component")
      .def(py::init<py::object>())
      .def("get_in_port_value", &Component::get_in_port_value)
      .def("get_out_port_value", &Component::get_out_port_value)
      .def("get_input", &Component::get_input)
      .def("get_output", &Component::get_output);

  m.def("connect", &brica::connect<Component>);

  py::class_<brica::Timing>(m, "Timing")
      .def(py::init<brica::Time, brica::Time, brica::Time>());

  py::class_<brica::VirtualTimeScheduler>(m, "VirtualTimeScheduler")
      .def(py::init<>())
      .def("add_component", &brica::VirtualTimeScheduler::add_component)
      .def("step", &brica::VirtualTimeScheduler::step);
}
