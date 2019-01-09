#include <pybind11/pybind11.h>
#include "brica/brica.hpp"
#include "pfifo/pfifo.hpp"

#include <sstream>
#include <string>

namespace py = pybind11;

using Buffer = brica::Buffer;
using Dict = brica::Dict;
using Port = brica::Port<Buffer>;
using Ports = brica::Ports<Buffer>;

void recv_dict(pfifo::reader* reader, Dict& dict) {
  std::string key;
  while ((key = reader->reads()).length() > 0) {
    dict[key] = reader->read<brica::Buffer>();
  }
}

void send_dict(pfifo::writer* writer, Dict& dict) {
  for (std::size_t i = 0; i < dict.size(); ++i) {
    Buffer val = dict.index(i);
    writer->write(dict.key(i));
    writer->write(val);
  }
  writer->write("");
}

class Component final : public brica::IComponent {
 public:
  Component(py::object f) {
    std::stringstream ss;
    ss << static_cast<void*>(this);
    std::string name = ss.str();

    py::object Dispatcher = py::module::import("_brica").attr("Dispatcher");
    dispatcher = Dispatcher(name, f);

    reader = new pfifo::reader(name + "p", 0644);
    writer = new pfifo::writer(name + "c", 0644);
  }

  virtual ~Component() {
    writer->write("0");
    delete reader;
    delete writer;
  }

  void make_in_port(std::string name) {
    inputs.try_emplace(name);
    in_ports.try_emplace(name, std::make_shared<Port>());
  }

  void make_out_port(std::string name) {
    outputs.try_emplace(name);
    out_ports.try_emplace(name, std::make_shared<Port>());
  }

  std::shared_ptr<Port>& get_in_port(std::string name) {
    return in_ports.at(name);
  }

  std::shared_ptr<Port>& get_out_port(std::string name) {
    return out_ports.at(name);
  }

  py::object get_in_port_value(std::string name) {
    py::module reduction = py::module::import("multiprocessing.reduction");
    py::object pickle = reduction.attr("ForkingPickler");
    Buffer buffer = in_ports.at(name)->get();
    std::string tmp(buffer.begin(), buffer.end());
    if (tmp.length() > 0) {
      return pickle.attr("loads")(py::bytes(tmp));
    }
    return py::none();
  }

  py::object get_out_port_value(std::string name) {
    py::module reduction = py::module::import("multiprocessing.reduction");
    py::object pickle = reduction.attr("ForkingPickler");
    Buffer buffer = out_ports.at(name)->get();
    std::string tmp(buffer.begin(), buffer.end());
    if (tmp.length() > 0) {
      return pickle.attr("loads")(py::bytes(tmp));
    }
    return py::none();
  }

  py::object get_input(std::string name) {
    py::module reduction = py::module::import("multiprocessing.reduction");
    py::object pickle = reduction.attr("ForkingPickler");
    Buffer buffer = inputs.at(name);
    std::string tmp(buffer.begin(), buffer.end());
    if (tmp.length() > 0) {
      return pickle.attr("loads")(py::bytes(tmp));
    }
    return py::none();
  }

  py::object get_output(std::string name) {
    py::module reduction = py::module::import("multiprocessing.reduction");
    py::object pickle = reduction.attr("ForkingPickler");
    Buffer buffer = outputs.at(name);
    std::string tmp(buffer.begin(), buffer.end());
    if (tmp.length() > 0) {
      return pickle.attr("loads")(py::bytes(tmp));
    }
    return py::none();
  }

  py::object get_object() {
    py::module reduction = py::module::import("multiprocessing.reduction");
    py::object pickle = reduction.attr("ForkingPickler");
    writer->write("2");
    std::string s = reader->reads();
    return pickle.attr("loads")(py::bytes(s));
  }

  void connect(Component& target, std::string out, std::string in) {
    in_ports.at(in) = target.out_ports.at(out);
  }

  void collect() {
    for (std::size_t i = 0; i < inputs.size(); ++i) {
      inputs.index(i) = in_ports.index(i)->get();
    }
  }

  void execute() {
    writer->write("1");
    send_dict(writer, inputs);
    recv_dict(reader, outputs);
  }

  void expose() {
    for (std::size_t i = 0; i < outputs.size(); ++i) {
      out_ports.index(i)->set(outputs.index(i));
    }
  }

 private:
  py::object dispatcher;
  pfifo::reader* reader;
  pfifo::writer* writer;

  Dict inputs;
  Dict outputs;

  Ports in_ports;
  Ports out_ports;
};

PYBIND11_MODULE(brica, m) {
  py::class_<brica::IComponent>(m, "IComponent")
      .def("collect", &brica::IComponent::collect)
      .def("execute", &brica::IComponent::execute)
      .def("expose", &brica::IComponent::expose);

  py::class_<Component, brica::IComponent>(m, "Component")
      .def(py::init<py::object>())
      .def("make_in_port", &Component::make_in_port)
      .def("make_out_port", &Component::make_out_port)
      .def("get_in_port_value", &Component::get_in_port_value)
      .def("get_out_port_value", &Component::get_out_port_value)
      .def("get_input", &Component::get_input)
      .def("get_output", &Component::get_output)
      .def("get_object", &Component::get_object);

  m.def("connect", &brica::connect<Component>);

  py::class_<brica::Timing>(m, "Timing")
      .def(py::init<brica::Time, brica::Time, brica::Time>());

  py::class_<brica::VirtualTimePhasedScheduler>(m, "VirtualTimePhasedScheduler")
      .def(py::init<>())
      .def("add_component", &brica::VirtualTimePhasedScheduler::add_component)
      .def("step", &brica::VirtualTimePhasedScheduler::step);

  py::class_<brica::VirtualTimeScheduler>(m, "VirtualTimeScheduler")
      .def(py::init<>())
      .def("add_component", &brica::VirtualTimeScheduler::add_component)
      .def("step", &brica::VirtualTimeScheduler::step);
}
